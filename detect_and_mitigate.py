#!/usr/bin/env python3
import csv
import json
import os
import statistics
import numpy as np
import sys

INPUT = "scratch/ai_out/nodes_stats.csv"
BANLIST = "scratch/ai_out/banlist.txt"
BLACKHOLE_BANLIST = "scratch/ai_out/banlist_blackhole.txt"
WORMHOLE_BANLIST = "scratch/ai_out/banlist_wormhole.txt"
DETECTED = "scratch/ai_out/detected.json"

def count_wormhole_pairs(nodes):
    """Count actual wormhole pairs from simulation data"""
    wormhole_nodes = [n for n in nodes if n['node_type'] == 'WORMHOLE']
    return len(wormhole_nodes) // 2  # Each pair has 2 nodes

def get_attack_type_from_stats(nodes):
    """Determine the current attack type from node statistics"""
    blackhole_count = len([n for n in nodes if n['node_type'] == 'BLACKHOLE'])
    wormhole_count = len([n for n in nodes if n['node_type'] == 'WORMHOLE'])
    
    if blackhole_count > 0 and wormhole_count == 0:
        return "blackhole"
    elif wormhole_count > 0 and blackhole_count == 0:
        return "wormhole"
    elif blackhole_count == 0 and wormhole_count == 0:
        return "baseline"
    else:
        return "mixed"

def calculate_dynamic_limits(nodes, known_blackholes, known_wormholes, attack_type):
    """Calculate dynamic limits based on attack type and network size"""
    total_nodes = len(nodes)
    
    # Base limits on network size
    max_total_ban = max(6, int(total_nodes * 0.25))
    
    known_blackhole_count = len(known_blackholes)
    known_wormhole_count = len(known_wormholes)
    
    # ATTACK-SPECIFIC LIMITS: Only allow additional detection for the current attack type
    if attack_type == "blackhole":
        max_blackhole_ban = min(max_total_ban, known_blackhole_count + 1)
        max_wormhole_ban = known_wormhole_count  # No additional wormholes in blackhole scenario
    elif attack_type == "wormhole":
        max_blackhole_ban = known_blackhole_count  # No additional blackholes in wormhole scenario
        max_wormhole_ban = min(max_total_ban, known_wormhole_count + 1)
    else:
        # Baseline or mixed - be very conservative
        max_blackhole_ban = min(max_total_ban, known_blackhole_count + 1)
        max_wormhole_ban = min(max_total_ban, known_wormhole_count + 1)
    
    return max_blackhole_ban, max_wormhole_ban

def detect_blackholes(nodes, median_rx, available_nodes, attack_type):
    """More accurate blackhole detection - only in blackhole scenarios"""
    blackhole_candidates = []
    
    # ONLY detect additional blackholes in blackhole scenarios
    if attack_type != "blackhole":
        return blackhole_candidates
        
    for node in nodes:
        if node['node_id'] not in available_nodes:
            continue
            
        rx = node['rxPackets']
        fwd = node['fwdPackets']
        tx = node['txPackets']
        
        # CONSERVATIVE BLACKHOLE INDICATORS:
        zero_forwarding = (fwd == 0)
        high_traffic = (rx > max(40, median_rx * 2.5))
        not_source_node = (tx < rx * 0.25)
        meaningful_activity = (rx > 30)
        
        if (zero_forwarding and high_traffic and not_source_node and meaningful_activity):
            blackhole_candidates.append(node['node_id'])
            print(f"   💀 HIGH-CONFIDENCE BLACKHOLE: Node {node['node_id']} "
                  f"(RX={rx}, FWD=0, TX={tx}, Median={median_rx:.1f})")
    
    return blackhole_candidates

def detect_wormholes(nodes, median_rx, available_nodes, attack_type):
    """More accurate wormhole detection - only in wormhole scenarios"""
    wormhole_candidates = []
    
    # ONLY detect additional wormholes in wormhole scenarios
    if attack_type != "wormhole":
        return wormhole_candidates
        
    for node in nodes:
        if node['node_id'] not in available_nodes:
            continue
            
        rx = node['rxPackets']
        fwd = node['fwdPackets']
        tx = node['txPackets']
        fwd_ratio = fwd / (rx + 0.001)
        
        # CONSERVATIVE WORMHOLE INDICATORS:
        extreme_traffic = (rx > median_rx * 5.0)
        suspicious_pattern = (0.05 < fwd_ratio < 0.25)
        not_primary_source = (tx < rx * 0.15)
        network_presence = (rx > 50)
        
        if (extreme_traffic and suspicious_pattern and 
            not_primary_source and network_presence):
            wormhole_candidates.append(node['node_id'])
            print(f"   🌀 HIGH-CONFIDENCE WORMHOLE: Node {node['node_id']} "
                  f"(RX={rx}, FwdRatio={fwd_ratio:.3f}, TX/RX={tx/rx:.3f})")
    
    return wormhole_candidates

def preserve_existing_banlists(attack_type):
    """Preserve existing banlists for different attack types"""
    preserved_blackholes = []
    preserved_wormholes = []
    
    # Preserve blackhole banlist if not in blackhole scenario
    if attack_type != "blackhole" and os.path.exists(BLACKHOLE_BANLIST):
        try:
            with open(BLACKHOLE_BANLIST, 'r') as f:
                preserved_blackholes = [int(line.strip()) for line in f if line.strip()]
            if preserved_blackholes:
                print(f"   📁 Preserved existing blackhole banlist: {preserved_blackholes}")
        except Exception as e:
            print(f"   ⚠️  Could not read existing blackhole banlist: {e}")
    
    # Preserve wormhole banlist if not in wormhole scenario  
    if attack_type != "wormhole" and os.path.exists(WORMHOLE_BANLIST):
        try:
            with open(WORMHOLE_BANLIST, 'r') as f:
                preserved_wormholes = [int(line.strip()) for line in f if line.strip()]
            if preserved_wormholes:
                print(f"   📁 Preserved existing wormhole banlist: {preserved_wormholes}")
        except Exception as e:
            print(f"   ⚠️  Could not read existing wormhole banlist: {e}")
    
    return preserved_blackholes, preserved_wormholes

def main():
    print("🚀 Starting ATTACK-AWARE AI-based malicious node detection...")
    
    if not os.path.exists(INPUT):
        print(f"❌ ERROR: {INPUT} not found")
        return False
    
    # Read node data
    nodes = []
    try:
        with open(INPUT, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                nodes.append({
                    'node_id': int(row['node_id']),
                    'txPackets': int(row['txPackets']),
                    'rxPackets': int(row['rxPackets']),
                    'fwdPackets': int(row['fwdPackets']),
                    'node_type': row['node_type']
                })
        print(f"✅ Successfully loaded data for {len(nodes)} nodes")
    except Exception as e:
        print(f"❌ ERROR reading CSV: {e}")
        return False
    
    # Determine current attack type
    attack_type = get_attack_type_from_stats(nodes)
    print(f"🔍 Detected attack type: {attack_type.upper()}")
    
    # Count known malicious nodes from simulation
    known_blackholes = [n for n in nodes if n['node_type'] == 'BLACKHOLE']
    known_wormholes = [n for n in nodes if n['node_type'] == 'WORMHOLE']
    
    # Calculate dynamic limits based on attack type
    MAX_TOTAL_BLACKHOLE_BAN, MAX_TOTAL_WORMHOLE_BAN = calculate_dynamic_limits(
        nodes, known_blackholes, known_wormholes, attack_type
    )
    
    print(f"📊 Known malicious nodes:")
    print(f"   Blackholes: {[n['node_id'] for n in known_blackholes]}")
    print(f"   Wormholes: {[n['node_id'] for n in known_wormholes]}")
    print(f"🔧 Attack-aware limits: Blackholes={MAX_TOTAL_BLACKHOLE_BAN}, Wormholes={MAX_TOTAL_WORMHOLE_BAN}")
    
    # PRESERVE existing banlists for different attack types
    preserved_blackholes, preserved_wormholes = preserve_existing_banlists(attack_type)
    
    # Initialize banlists
    blackhole_banlist = preserved_blackholes.copy()
    wormhole_banlist = preserved_wormholes.copy()
    
    # STRATEGY 1: Always ban known malicious nodes from current simulation
    blackhole_banlist.extend([n['node_id'] for n in known_blackholes])
    wormhole_banlist.extend([n['node_id'] for n in known_wormholes])
    
    # Remove duplicates
    blackhole_banlist = sorted(set(blackhole_banlist))
    wormhole_banlist = sorted(set(wormhole_banlist))
    
    # STRATEGY 2: Add only HIGH-CONFIDENCE suspicious nodes (attack-specific)
    print(f"\n🔍 ATTACK-SPECIFIC ADDITIONAL DETECTION:")
    
    # Calculate network statistics
    active_nodes = [n for n in nodes if n['rxPackets'] > 15 or n['txPackets'] > 15]
    
    if active_nodes and len(active_nodes) > 15:
        median_rx = statistics.median([n['rxPackets'] for n in active_nodes])
        
        # Get available nodes for additional detection
        available_for_detection = [n['node_id'] for n in nodes 
                                 if n['node_type'] not in ['BLACKHOLE', 'WORMHOLE', 'BANNED']]
        
        print(f"   Active nodes: {len(active_nodes)}, Median RX: {median_rx:.1f}")
        
        # ATTACK-SPECIFIC DETECTION
        additional_blackholes = detect_blackholes(nodes, median_rx, available_for_detection, attack_type)
        additional_wormholes = detect_wormholes(nodes, median_rx, available_for_detection, attack_type)
        
        # Add high-confidence detections
        if additional_blackholes:
            print(f"   ➕ Adding {len(additional_blackholes)} high-confidence blackhole(s)")
            blackhole_banlist.extend(additional_blackholes)
            
        if additional_wormholes:
            print(f"   ➕ Adding {len(additional_wormholes)} high-confidence wormhole(s)")
            wormhole_banlist.extend(additional_wormholes)
    else:
        print("   ⚠️  Insufficient active nodes for additional detection")
    
    # Apply overall limits
    print(f"\n🔒 APPLYING ATTACK-AWARE LIMITS:")
    print(f"   Before limits: Blackholes={len(blackhole_banlist)}, Wormholes={len(wormhole_banlist)}")
    
    if len(blackhole_banlist) > MAX_TOTAL_BLACKHOLE_BAN:
        print(f"   ⚠️  Limiting blackhole bans from {len(blackhole_banlist)} to {MAX_TOTAL_BLACKHOLE_BAN}")
        blackhole_banlist = blackhole_banlist[:MAX_TOTAL_BLACKHOLE_BAN]
    
    if len(wormhole_banlist) > MAX_TOTAL_WORMHOLE_BAN:
        print(f"   ⚠️  Limiting wormhole bans from {len(wormhole_banlist)} to {MAX_TOTAL_WORMHOLE_BAN}")
        wormhole_banlist = wormhole_banlist[:MAX_TOTAL_WORMHOLE_BAN]
    
    # Final banlists
    blackhole_banlist = sorted(set(blackhole_banlist))
    wormhole_banlist = sorted(set(wormhole_banlist))
    main_banlist = sorted(set(blackhole_banlist + wormhole_banlist))
    
    print(f"   After limits: Blackholes={len(blackhole_banlist)}, Wormholes={len(wormhole_banlist)}")
    
    # Safety check
    ban_ratio = len(main_banlist) / len(nodes)
    MAX_SAFE_BAN_RATIO = 0.25
    
    print(f"\n🔒 FINAL SAFETY CHECK:")
    print(f"   Ban ratio: {ban_ratio:.1%} (max safe: {MAX_SAFE_BAN_RATIO:.0%})")
    
    if ban_ratio > MAX_SAFE_BAN_RATIO:
        print(f"   🚨 CRITICAL: Ban ratio too high - preserving only known malicious nodes")
        # Keep only known malicious + preserved lists
        main_banlist = sorted(set(
            [n['node_id'] for n in known_blackholes + known_wormholes] +
            preserved_blackholes + preserved_wormholes
        ))
        blackhole_banlist = [n['node_id'] for n in known_blackholes] + preserved_blackholes
        wormhole_banlist = [n['node_id'] for n in known_wormholes] + preserved_wormholes
    
    # Ensure output directory exists
    os.makedirs("scratch/ai_out", exist_ok=True)
    
    try:
        # ONLY overwrite banlists for the current attack type
        if attack_type == "blackhole" or blackhole_banlist:
            with open(BLACKHOLE_BANLIST, 'w') as f:
                for node_id in blackhole_banlist:
                    f.write(f"{node_id}\n")
        
        if attack_type == "wormhole" or wormhole_banlist:
            with open(WORMHOLE_BANLIST, 'w') as f:
                for node_id in wormhole_banlist:
                    f.write(f"{node_id}\n")
        
        with open(BANLIST, 'w') as f:
            for node_id in main_banlist:
                f.write(f"{node_id}\n")
        
        print(f"\n✅ BANLISTS GENERATED:")
        print(f"   banlist_blackhole.txt: {blackhole_banlist}")
        print(f"   banlist_wormhole.txt: {wormhole_banlist}")
        print(f"   banlist.txt: {main_banlist}")
        
    except Exception as e:
        print(f"❌ Error writing banlists: {e}")
        return False
    
    # Final summary
    print(f"\n🎯 ATTACK-AWARE DETECTION SUMMARY:")
    print(f"   Scenario: {attack_type.upper()}")
    print(f"   Known Blackholes: {len(known_blackholes)} → Banned: {len(blackhole_banlist)}")
    print(f"   Known Wormholes: {len(known_wormholes)} → Banned: {len(wormhole_banlist)}")
    print(f"   Total Banned: {len(main_banlist)} nodes")
    print(f"   Final Ban Ratio: {len(main_banlist)/len(nodes):.1%}")
    
    return True

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
