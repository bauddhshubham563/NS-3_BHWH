#!/bin/bash

# Enhanced MANET AI Security Project Runner
cd ~/ns-allinone-3.39/ns-3.39 || exit 1

echo "=========================================="
echo "=== AI-Based MANET Security Experiment ==="
echo "=========================================="

# Configuration
SIM_TIME=50
NODES=50
AREA=800
SEED_BASE=1000

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Helper functions
print_step() {
    echo -e "${BLUE}=== $1 ===${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

check_file_exists() {
    if [[ ! -f "$1" ]]; then
        print_error "Missing required file: $1"
        return 1
    fi
    return 0
}

check_simulation_output() {
    local output_dir="scratch/ai_out"
    local required_files=("nodes_stats.csv" "pdr_results.txt")
    
    for file in "${required_files[@]}"; do
        if [[ ! -f "$output_dir/$file" ]]; then
            print_warning "Missing output file: $output_dir/$file"
            return 1
        fi
    done
    
    # Check if nodes_stats.csv has reasonable data
    if [[ -f "$output_dir/nodes_stats.csv" ]]; then
        local line_count=$(wc -l < "$output_dir/nodes_stats.csv")
        if [[ $line_count -lt 3 ]]; then
            print_warning "nodes_stats.csv has insufficient data ($line_count lines)"
            return 1
        fi
    fi
    
    return 0
}

run_simulation() {
    local phase_name="$1"
    local command="$2"
    local expected_output="$3"
    
    print_step "$phase_name"
    echo "Command: $command"
    echo "Start: $(date '+%H:%M:%S')"
    
    # Run simulation with timeout
    timeout 360s ./ns3 run "$command" --no-build
    local exit_code=$?
    
    if [[ $exit_code -eq 0 ]]; then
        if check_simulation_output; then
            print_success "$phase_name completed successfully"
            return 0
        else
            print_warning "$phase_name completed but with output issues"
            return 1
        fi
    elif [[ $exit_code -eq 124 ]]; then
        print_error "$phase_name timed out after 180 seconds"
        return 1
    else
        print_error "$phase_name failed (exit code: $exit_code)"
        return 1
    fi
}

run_ai_detection() {
    print_step "AI-BASED MALICIOUS NODE DETECTION"
    
    # Check if we have the required input file
    if ! check_file_exists "scratch/ai_out/nodes_stats.csv"; then
        print_error "Cannot run AI detection - missing nodes_stats.csv"
        return 1
    fi
    
    echo "Running AI detection algorithm..."
    if python3 scratch/detect_and_mitigate.py; then
        print_success "AI detection completed"
        
        # Display banlist contents
        echo "=== GENERATED BANLISTS ==="
        for banlist in "banlist_blackhole.txt" "banlist_wormhole.txt" "banlist.txt"; do
            local file="scratch/ai_out/$banlist"
            if [[ -f "$file" ]]; then
                local nodes=$(tr '\n' ' ' < "$file")
                echo "📄 $banlist: $nodes"
            else
                print_warning "Missing $banlist"
            fi
        done
        
        return 0
    else
        print_error "AI detection failed"
        return 1
    fi
}

clean_previous_results() {
    print_step "CLEANING PREVIOUS RESULTS"
    rm -rf scratch/ai_out
    mkdir -p scratch/ai_out
    print_success "Cleanup completed"
}

generate_final_report() {
    print_step "GENERATING FINAL ANALYSIS REPORT"
    
    if [[ ! -f "scratch/ai_out/pdr_results.txt" ]]; then
        print_error "No PDR results found for analysis"
        return 1
    fi
    
    # Generate enhanced plots
    if python3 scratch/plot_ai_results.py; then
        print_success "Enhanced analysis plots generated"
        
        # Display summary statistics
        echo "=== EXPERIMENT SUMMARY ==="
        if [[ -f "scratch/ai_out/pdr_results.txt" ]]; then
            echo "📊 PDR Results:"
            cat scratch/ai_out/pdr_results.txt
        fi
        
        # Check for generated plots
        if [[ -f "scratch/ai_out/ai_manet_analysis_latest.png" ]]; then
            echo "📈 Analysis plot: scratch/ai_out/ai_manet_analysis_latest.png"
        fi
        
        return 0
    else
        print_error "Failed to generate analysis plots"
        return 1
    fi
}

# Main experiment execution
main() {
    local overall_success=true
    
    clean_previous_results
    
    echo "=========================================="
    echo "        EXPERIMENT CONFIGURATION"
    echo "=========================================="
    echo "Nodes: $NODES"
    echo "Simulation Time: $SIM_TIME seconds"
    echo "Area: ${AREA}x${AREA}m"
    echo "Seed Base: $SEED_BASE"
    echo "=========================================="
    echo ""
    
    # PHASE 1: BASELINE (Normal Network)
    if run_simulation "1. BASELINE - NORMAL NETWORK" \
        "scratch/manet_ai_security --routing=AODV --attack=none --nNodes=$NODES --simTime=$SIM_TIME --area=$AREA --seed=$((SEED_BASE+1))" \
        "baseline"; then
        print_success "Baseline established"
    else
        print_error "Baseline phase failed"
        overall_success=false
    fi
    
    # PHASE 2: BLACKHOLE ATTACK
    if run_simulation "2. BLACKHOLE ATTACK" \
        "scratch/manet_ai_security --routing=AODV --attack=blackhole --nNodes=$NODES --simTime=$SIM_TIME --nBlackholes=3 --area=$AREA --seed=$((SEED_BASE+2))" \
        "blackhole_attack"; then
        print_success "Blackhole attack phase completed"
    else
        print_warning "Blackhole attack phase had issues - continuing"
    fi
    
    # PHASE 3: AI DETECTION AFTER BLACKHOLE
    if run_ai_detection; then
        print_success "AI detection after blackhole completed"
    else
        print_warning "AI detection after blackhole had issues"
    fi
    
    # PHASE 4: BLACKHOLE MITIGATION
    if [[ -f "scratch/ai_out/banlist_blackhole.txt" ]] && [[ -s "scratch/ai_out/banlist_blackhole.txt" ]]; then
        if run_simulation "4. BLACKHOLE MITIGATION" \
            "scratch/manet_ai_security --routing=AODV --attack=blackhole --nNodes=$NODES --simTime=$SIM_TIME --nBlackholes=3 --banlist=scratch/ai_out/banlist_blackhole.txt --area=$AREA --seed=$((SEED_BASE+2))" \
            "blackhole_mitigation"; then
            print_success "Blackhole mitigation completed"
        else
            print_warning "Blackhole mitigation had issues"
        fi
    else
        print_warning "Skipping blackhole mitigation - no blackhole nodes detected"
        # Run without banlist to maintain experiment flow
        run_simulation "4. BLACKHOLE MITIGATION (No Ban)" \
            "scratch/manet_ai_security --routing=AODV --attack=blackhole --nNodes=$NODES --simTime=$SIM_TIME --nBlackholes=3 --area=$AREA --seed=$((SEED_BASE+3))" \
            "blackhole_no_mitigation"
    fi
    
    # PHASE 5: WORMHOLE ATTACK
    if run_simulation "5. WORMHOLE ATTACK" \
        "scratch/manet_ai_security --routing=AODV --attack=wormhole --nNodes=$NODES --simTime=$SIM_TIME --nWormholes=3 --area=$AREA --seed=$((SEED_BASE+4))" \
        "wormhole_attack"; then
        print_success "Wormhole attack phase completed"
    else
        print_warning "Wormhole attack phase had issues - continuing"
    fi
    
    # PHASE 6: AI DETECTION AFTER WORMHOLE
    if run_ai_detection; then
        print_success "AI detection after wormhole completed"
    else
        print_warning "AI detection after wormhole had issues"
    fi
    
    # PHASE 7: WORMHOLE MITIGATION
    if [[ -f "scratch/ai_out/banlist_wormhole.txt" ]] && [[ -s "scratch/ai_out/banlist_wormhole.txt" ]]; then
        if run_simulation "7. WORMHOLE MITIGATION" \
            "scratch/manet_ai_security --routing=AODV --attack=wormhole --nNodes=$NODES --simTime=$SIM_TIME --nWormholes=3 --banlist=scratch/ai_out/banlist_wormhole.txt --area=$AREA --seed=$((SEED_BASE+4))" \
            "wormhole_mitigation"; then
            print_success "Wormhole mitigation completed"
        else
            print_warning "Wormhole mitigation had issues"
        fi
    else
        print_warning "Skipping wormhole mitigation - no wormhole nodes detected"
        # Run without banlist to maintain experiment flow
        run_simulation "7. WORMHOLE MITIGATION (No Ban)" \
            "scratch/manet_ai_security --routing=AODV --attack=wormhole --nNodes=$NODES --simTime=$SIM_TIME --nWormholes=3 --area=$AREA --seed=$((SEED_BASE+5))" \
            "wormhole_no_mitigation"
    fi
    
    # FINAL ANALYSIS AND REPORTING
    if generate_final_report; then
        print_success "Final report generated successfully"
    else
        print_error "Final report generation failed"
        overall_success=false
    fi
    
    # FINAL SUMMARY
    echo ""
    echo "=========================================="
    if $overall_success; then
        print_success "🎉 EXPERIMENT COMPLETED SUCCESSFULLY!"
    else
        print_warning "⚠️  EXPERIMENT COMPLETED WITH SOME ISSUES"
    fi
    echo "=========================================="
    
    # Display key output files
    echo ""
    echo "=== KEY OUTPUT FILES ==="
    for file in "scratch/ai_out/pdr_results.txt" \
                "scratch/ai_out/ai_manet_analysis_latest.png" \
                "scratch/ai_out/nodes_stats.csv"; do
        if [[ -f "$file" ]]; then
            echo "📄 $(basename "$file")"
        fi
    done
    
    return $overall_success
}

# Build the project first
print_step "BUILDING NS-3 PROJECT"
if ./ns3 build scratch/manet_ai_security.cc; then
    print_success "Build completed successfully"
    # Run the main experiment
    main
else
    print_error "Build failed - cannot run experiment"
    exit 1
fi
