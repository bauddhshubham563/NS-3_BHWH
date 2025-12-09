#!/usr/bin/env python3
import matplotlib.pyplot as plt
import os
import numpy as np
import datetime
import pandas as pd
from collections import defaultdict

def calculate_improvement_metrics(data):
    """Calculate improvement metrics across scenarios"""
    metrics = {}
    
    # Group by attack type and scenario
    scenarios = defaultdict(list)
    for entry in data:
        key = (entry['AttackType'], entry['Scenario'])
        scenarios[key].append(entry)
    
    # Calculate baseline (normal network)
    baseline_pdr = None
    for entry in data:
        if entry['AttackType'] == 'none' and entry['Scenario'] == 'BASELINE':
            baseline_pdr = entry['PDR']
            break
    
    if baseline_pdr is None:
        print("⚠️  No baseline scenario found")
        return metrics
    
    # Calculate metrics for each scenario
    for (attack_type, scenario), entries in scenarios.items():
        if not entries:
            continue
            
        # Safe average calculation
        pdrs = [e['PDR'] for e in entries if not np.isnan(e['PDR'])]
        if not pdrs:
            continue
            
        avg_pdr = np.mean(pdrs)
        
        if scenario == 'ATTACK':
            attack_impact = ((baseline_pdr - avg_pdr) / baseline_pdr) * 100 if baseline_pdr > 0 else 0
            metrics[(attack_type, scenario)] = {
                'avg_pdr': avg_pdr,
                'attack_impact': max(0, attack_impact),
                'recovery_rate': 0
            }
        elif scenario == 'MITIGATION':
            # Find corresponding attack PDR
            attack_pdr = None
            for (at, sc), e_list in scenarios.items():
                if at == attack_type and sc == 'ATTACK':
                    attack_pdrs = [x['PDR'] for x in e_list if not np.isnan(x['PDR'])]
                    if attack_pdrs:
                        attack_pdr = np.mean(attack_pdrs)
                    break
            
            if attack_pdr and attack_pdr > 0:
                mitigation_gain = ((avg_pdr - attack_pdr) / attack_pdr) * 100
                recovery_rate = (avg_pdr / baseline_pdr) * 100 if baseline_pdr > 0 else 0
                metrics[(attack_type, scenario)] = {
                    'avg_pdr': avg_pdr,
                    'mitigation_gain': mitigation_gain,
                    'recovery_rate': recovery_rate
                }
    
    return metrics

def plot_ai_results():
    fn = "scratch/ai_out/pdr_results.txt"
    
    if not os.path.exists(fn):
        print(f"ERROR: Cannot find {fn}")
        return False

    data = []
    labels = []
    attacks = []
    scenarios = []
    tx_values = []
    rx_values = []
    pdr_values = []
    
    print("📊 Parsing simulation results...")
    
    with open(fn) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
                
            try:
                # Parse the line
                parts = line.split()
                data_dict = {}
                for part in parts:
                    if '=' in part:
                        key, value = part.split('=', 1)
                        data_dict[key] = value
                
                routing = data_dict.get('Routing', 'UNKNOWN')
                attack = data_dict.get('AttackType', 'none')
                scenario = data_dict.get('Scenario', 'UNKNOWN')
                tx = int(data_dict.get('Tx', 0))
                rx = int(data_dict.get('Rx', 0))
                
                # Safe PDR calculation
                pdr_str = data_dict.get('PDR', '0')
                try:
                    pdr = float(pdr_str) if pdr_str != 'nan' else 0.0
                except ValueError:
                    pdr = 0.0
                
                nodes = int(data_dict.get('Nodes', 0))
                blackholes = int(data_dict.get('Blackholes', 0))
                wormholes = int(data_dict.get('Wormholes', 0))
                banned = int(data_dict.get('BannedNodes', 0))
                
                # Safe efficiency calculation
                efficiency_str = data_dict.get('Efficiency', '0')
                try:
                    efficiency = float(efficiency_str) if efficiency_str != 'nan' else 0.0
                except ValueError:
                    efficiency = 0.0
                    
                active_flows = int(data_dict.get('ActiveFlows', 0))
                
                # Store data for analysis
                entry = {
                    'Routing': routing,
                    'AttackType': attack,
                    'Scenario': scenario,
                    'Tx': tx,
                    'Rx': rx,
                    'PDR': pdr,
                    'Nodes': nodes,
                    'Blackholes': blackholes,
                    'Wormholes': wormholes,
                    'BannedNodes': banned,
                    'Efficiency': efficiency,
                    'ActiveFlows': active_flows
                }
                data.append(entry)
                
                # Create descriptive label
                label = f"{scenario}\n{attack}"
                if blackholes > 0:
                    label += f"\n{blackholes}BH"
                if wormholes > 0:
                    label += f"\n{wormholes}WH"
                if banned > 0:
                    label += f"\n{banned}Ban"
                
                labels.append(label)
                pdr_values.append(pdr)
                attacks.append(attack)
                scenarios.append(scenario)
                tx_values.append(tx)
                rx_values.append(rx)
                
                print(f"📈 Parsed: {scenario}-{attack} PDR={pdr:.3f} "
                      f"Tx={tx} Rx={rx} Flows={active_flows}")
                
            except (ValueError, KeyError) as e:
                print(f"⚠️  Error parsing line: {e}")
                continue

    if not pdr_values:
        print("❌ ERROR: No valid data found")
        return False

    # Filter out NaN values for calculations
    valid_indices = [i for i, pdr in enumerate(pdr_values) if not np.isnan(pdr)]
    if not valid_indices:
        print("❌ ERROR: No valid PDR values found")
        return False
        
    valid_pdr_values = [pdr_values[i] for i in valid_indices]
    valid_labels = [labels[i] for i in valid_indices]
    valid_attacks = [attacks[i] for i in valid_indices]
    valid_scenarios = [scenarios[i] for i in valid_indices]
    valid_tx_values = [tx_values[i] for i in valid_indices]
    valid_rx_values = [rx_values[i] for i in valid_indices]

    # Calculate improvement metrics
    improvement_metrics = calculate_improvement_metrics(data)
    
    # Create enhanced visualization
    fig = plt.figure(figsize=(18, 12))
    
    # Plot 1: PDR Comparison (Main Results)
    ax1 = plt.subplot2grid((3, 3), (0, 0), colspan=2)
    colors = []
    for attack, scenario in zip(valid_attacks, valid_scenarios):
        if scenario == 'BASELINE':
            colors.append('green')
        elif scenario == 'ATTACK':
            if attack == 'blackhole':
                colors.append('red')
            elif attack == 'wormhole':
                colors.append('blue')
            else:
                colors.append('orange')
        else:  # MITIGATION
            if attack == 'blackhole':
                colors.append('lightcoral')
            elif attack == 'wormhole':
                colors.append('lightblue')
            else:
                colors.append('lightgreen')
    
    bars = ax1.bar(valid_labels, valid_pdr_values, color=colors, alpha=0.8, edgecolor='black', linewidth=1.2)
    
    # Add value labels on bars
    for bar, pdr_val in zip(bars, valid_pdr_values):
        height = bar.get_height()
        ax1.text(bar.get_x() + bar.get_width()/2., height + 0.01,
                f'{pdr_val:.3f}', ha='center', va='bottom', fontweight='bold', fontsize=8)
    
    ax1.set_ylim(0, 1.1)
    ax1.set_ylabel('Packet Delivery Ratio (PDR)', fontsize=12, fontweight='bold')
    ax1.set_title('AI-Based MANET Security: PDR Comparison\n(Attacks vs Mitigation)', 
                 fontsize=14, fontweight='bold', pad=20)
    ax1.grid(True, alpha=0.3, axis='y')
    ax1.tick_params(axis='x', rotation=45, labelsize=8)
    
    # Add legend
    from matplotlib.patches import Patch
    legend_elements = [
        Patch(facecolor='green', alpha=0.8, label='Baseline (Normal)'),
        Patch(facecolor='red', alpha=0.8, label='Blackhole Attack'),
        Patch(facecolor='blue', alpha=0.8, label='Wormhole Attack'),
        Patch(facecolor='lightcoral', alpha=0.8, label='Blackhole Mitigation'),
        Patch(facecolor='lightblue', alpha=0.8, label='Wormhole Mitigation')
    ]
    ax1.legend(handles=legend_elements, loc='upper right', fontsize=9)
    
    # Plot 2: Throughput Analysis
    ax2 = plt.subplot2grid((3, 3), (0, 2))
    x_pos = np.arange(len(valid_labels))
    width = 0.35
    
    ax2.bar(x_pos - width/2, valid_tx_values, width, label='Transmitted', alpha=0.7, color='navy', edgecolor='black')
    ax2.bar(x_pos + width/2, valid_rx_values, width, label='Received', alpha=0.7, color='limegreen', edgecolor='black')
    
    ax2.set_xlabel('Scenarios', fontsize=10, fontweight='bold')
    ax2.set_ylabel('Number of Packets', fontsize=10, fontweight='bold')
    ax2.set_title('Packet Throughput Analysis', fontsize=12, fontweight='bold')
    ax2.set_xticks(x_pos)
    ax2.set_xticklabels([label.replace('\n', ' ') for label in valid_labels], 
                       rotation=45, ha='right', fontsize=7)
    ax2.legend(fontsize=9)
    ax2.grid(True, alpha=0.3)
    
    # Plot 3: Packet Loss Analysis
    ax3 = plt.subplot2grid((3, 3), (1, 0), colspan=2)
    packet_loss = [tx - rx for tx, rx in zip(valid_tx_values, valid_rx_values)]
    loss_rates = [loss/tx if tx > 0 else 0 for loss, tx in zip(packet_loss, valid_tx_values)]
    
    loss_bars = ax3.bar(valid_labels, loss_rates, color='red', alpha=0.7, edgecolor='darkred')
    
    # Add value labels
    for bar, loss_rate in zip(loss_bars, loss_rates):
        if loss_rate > 0:
            height = bar.get_height()
            ax3.text(bar.get_x() + bar.get_width()/2., height + 0.01,
                    f'{loss_rate:.1%}', ha='center', va='bottom', fontweight='bold', fontsize=8)
    
    ax3.set_ylabel('Packet Loss Rate', fontsize=10, fontweight='bold')
    ax3.set_title('Packet Loss Analysis by Scenario', fontsize=12, fontweight='bold')
    ax3.grid(True, alpha=0.3, axis='y')
    ax3.tick_params(axis='x', rotation=45, labelsize=8)
    ax3.set_ylim(0, 1.0)
    
    # Plot 4: Improvement Metrics
    ax4 = plt.subplot2grid((3, 3), (1, 2))
    
    if improvement_metrics:
        attack_types = []
        attack_impacts = []
        mitigation_gains = []
        
        for (attack_type, scenario), metrics in improvement_metrics.items():
            if scenario == 'ATTACK' and attack_type != 'none':
                attack_types.append(attack_type)
                attack_impacts.append(metrics['attack_impact'])
            elif scenario == 'MITIGATION' and attack_type != 'none':
                # Find corresponding mitigation gain
                for (at, sc), m in improvement_metrics.items():
                    if at == attack_type and sc == 'MITIGATION':
                        mitigation_gains.append(m['mitigation_gain'])
                        break
        
        if attack_types and attack_impacts and mitigation_gains and len(attack_types) == len(mitigation_gains):
            x = np.arange(len(attack_types))
            width = 0.35
            
            ax4.bar(x - width/2, attack_impacts, width, label='Attack Impact (%)', 
                   color='red', alpha=0.7, edgecolor='darkred')
            ax4.bar(x + width/2, mitigation_gains, width, label='Mitigation Gain (%)', 
                   color='green', alpha=0.7, edgecolor='darkgreen')
            
            ax4.set_xlabel('Attack Type', fontsize=10, fontweight='bold')
            ax4.set_ylabel('Percentage (%)', fontsize=10, fontweight='bold')
            ax4.set_title('Attack Impact vs Mitigation Gain', fontsize=12, fontweight='bold')
            ax4.set_xticks(x)
            ax4.set_xticklabels(attack_types, fontsize=10)
            ax4.legend(fontsize=9)
            ax4.grid(True, alpha=0.3)
        else:
            ax4.text(0.5, 0.5, 'Insufficient data\nfor improvement metrics', 
                    ha='center', va='center', transform=ax4.transAxes, fontsize=12)
            ax4.set_title('Improvement Metrics', fontsize=12, fontweight='bold')
    else:
        ax4.text(0.5, 0.5, 'No improvement metrics\navailable', 
                ha='center', va='center', transform=ax4.transAxes, fontsize=12)
        ax4.set_title('Improvement Metrics', fontsize=12, fontweight='bold')
    
    # Plot 5: Scenario Efficiency
    ax5 = plt.subplot2grid((3, 3), (2, 0), colspan=3)
    
    efficiencies = [entry.get('Efficiency', 0) for entry in data if not np.isnan(entry.get('Efficiency', 0))]
    if efficiencies and any(efficiencies):
        efficiency_labels = [valid_labels[i] for i in range(len(valid_labels)) if not np.isnan(data[i].get('Efficiency', 0))]
        efficiency_bars = ax5.bar(efficiency_labels, efficiencies, color='purple', alpha=0.7, edgecolor='darkviolet')
        
        for bar, eff in zip(efficiency_bars, efficiencies):
            height = bar.get_height()
            ax5.text(bar.get_x() + bar.get_width()/2., height + 1,
                    f'{eff:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=8)
        
        ax5.set_ylabel('Network Efficiency (%)', fontsize=10, fontweight='bold')
        ax5.set_title('Overall Network Efficiency by Scenario', fontsize=12, fontweight='bold')
        ax5.grid(True, alpha=0.3, axis='y')
        ax5.tick_params(axis='x', rotation=45, labelsize=8)
    else:
        ax5.text(0.5, 0.5, 'No efficiency data\navailable', 
                ha='center', va='center', transform=ax5.transAxes, fontsize=12)
        ax5.set_title('Network Efficiency', fontsize=12, fontweight='bold')
    
    # Calculate and display comprehensive statistics
    avg_pdr = np.mean(valid_pdr_values) if valid_pdr_values else 0
    max_pdr = np.max(valid_pdr_values) if valid_pdr_values else 0
    min_pdr = np.min(valid_pdr_values) if valid_pdr_values else 0
    std_pdr = np.std(valid_pdr_values) if valid_pdr_values else 0
    
    # Summary text
    summary_text = f"""EXPERIMENT SUMMARY:
    • Total Scenarios: {len(valid_pdr_values)}
    • Average PDR: {avg_pdr:.3f} ± {std_pdr:.3f}
    • Best PDR: {max_pdr:.3f} | Worst PDR: {min_pdr:.3f}
    • Total Packets: Tx={sum(valid_tx_values):,} | Rx={sum(valid_rx_values):,}
    • Overall Delivery Rate: {sum(valid_rx_values)/sum(valid_tx_values)*100:.1f}%""" if sum(valid_tx_values) > 0 else "No packet data"
    
    # Add improvement metrics to summary
    if improvement_metrics:
        for (attack_type, scenario), metrics in improvement_metrics.items():
            if scenario == 'ATTACK' and attack_type != 'none':
                summary_text += f"\n    • {attack_type.upper()} Attack Impact: {metrics['attack_impact']:.1f}%"
            elif scenario == 'MITIGATION' and attack_type != 'none':
                summary_text += f"\n    • {attack_type.upper()} Mitigation Gain: {metrics['mitigation_gain']:.1f}%"
    
    plt.figtext(0.02, 0.02, summary_text, fontsize=9, 
                bbox=dict(boxstyle="round,pad=0.5", facecolor="lightgray", alpha=0.8),
                fontfamily='monospace')
    
    plt.tight_layout()
    plt.subplots_adjust(bottom=0.15)
    
    # Save with timestamp and latest version
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    output_file = f"scratch/ai_out/ai_manet_analysis_{timestamp}.png"
    
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    plt.savefig("scratch/ai_out/ai_manet_analysis_latest.png", dpi=300, bbox_inches='tight')
    
    print(f"✅ Enhanced analysis plot saved: {output_file}")
    print(f"📊 Statistical Summary:")
    print(f"   PDR Range: {min_pdr:.3f} - {max_pdr:.3f}")
    print(f"   Average PDR: {avg_pdr:.3f} ± {std_pdr:.3f}")
    print(f"   Total Packets: {sum(valid_tx_values):,} transmitted, {sum(valid_rx_values):,} received")
    
    # Detailed analysis by attack type
    print(f"\n📈 DETAILED ANALYSIS BY ATTACK TYPE:")
    attack_data = defaultdict(list)
    for entry in data:
        if not np.isnan(entry['PDR']):
            attack_data[entry['AttackType']].append(entry)
    
    for attack_type, entries in attack_data.items():
        if attack_type == 'none':
            continue
            
        attack_entries = [e for e in entries if e['Scenario'] == 'ATTACK' and not np.isnan(e['PDR'])]
        mitigation_entries = [e for e in entries if e['Scenario'] == 'MITIGATION' and not np.isnan(e['PDR'])]
        
        if attack_entries:
            avg_pdr_attack = np.mean([e['PDR'] for e in attack_entries])
            print(f"   {attack_type.upper()}:")
            print(f"     Attack PDR: {avg_pdr_attack:.3f} (n={len(attack_entries)})")
            
            if mitigation_entries:
                avg_pdr_mitigation = np.mean([e['PDR'] for e in mitigation_entries])
                improvement = ((avg_pdr_mitigation - avg_pdr_attack) / avg_pdr_attack) * 100
                print(f"     Mitigation PDR: {avg_pdr_mitigation:.3f} (n={len(mitigation_entries)})")
                print(f"     Improvement: {improvement:+.1f}%")
            else:
                print(f"     Mitigation PDR: No data available")
        else:
            print(f"   {attack_type.upper()}: No attack phase data available")
    
    return True

if __name__ == "__main__":
    success = plot_ai_results()
    if not success:
        print("❌ Failed to generate enhanced analysis plots")
        exit(1)
