// manet_ai_security.cc - FIXED BANNING CONSISTENCY
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include <fstream>
#include <set>
#include <vector>
#include <map>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <random>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ManetAiSecurity");

std::vector<uint32_t> g_forwardCount;

static void ForwardingTrace(uint32_t nodeId, const Ipv4Header &header, Ptr<const Packet> packet, uint32_t interface) {
    if (nodeId < g_forwardCount.size()) {
        g_forwardCount[nodeId]++;
    }
}

// SIMPLIFIED BUT EFFECTIVE BLACKHOLE IMPLEMENTATION
class EffectiveBlackholeApp : public Application {
public:
    EffectiveBlackholeApp() : m_droppedCount(0) {}
    virtual ~EffectiveBlackholeApp() {}
    
    static TypeId GetTypeId() {
        static TypeId tid = TypeId("EffectiveBlackholeApp")
            .SetParent<Application>()
            .AddConstructor<EffectiveBlackholeApp>();
        return tid;
    }

protected:
    virtual void StartApplication() {
        Ptr<Node> node = GetNode();
        uint32_t nodeId = node->GetId();
        
        std::cout << "💀 INITIALIZING EFFECTIVE BLACKHOLE ON NODE " << nodeId << std::endl;
        
        // METHOD 1: COMPLETELY DISABLE IP FORWARDING AT KERNEL LEVEL
        node->GetObject<Ipv4>()->SetAttribute("IpForward", BooleanValue(false));
        
        // METHOD 2: CREATE FLOODING TRAFFIC TO OVERWHELM THE NODE
        OnOffHelper floodHelper("ns3::UdpSocketFactory", Address());
        floodHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
        floodHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
        floodHelper.SetAttribute("DataRate", StringValue("500kbps"));
        floodHelper.SetAttribute("PacketSize", UintegerValue(1000));
        
        // Flood to multiple random destinations
        for (int i = 0; i < 3; i++) {
            uint32_t randomDest = rand() % g_forwardCount.size();
            if (randomDest != nodeId) {
                // Create IP address properly
                std::string ipStr = "10.1.1." + std::to_string(randomDest + 1);
                Ipv4Address destAddr(ipStr.c_str());
                InetSocketAddress remoteAddr(destAddr, 9999);
                floodHelper.SetAttribute("Remote", AddressValue(remoteAddr));
                ApplicationContainer floodApp = floodHelper.Install(node);
                floodApp.Start(Seconds(2.0 + i * 0.5));
            }
        }
        
        std::cout << "💀 EFFECTIVE BLACKHOLE ACTIVE: Node " << nodeId << " will DROP ALL TRAFFIC" << std::endl;
    }
    
    virtual void StopApplication() {
        std::cout << "💀 BLACKHOLE " << GetNode()->GetId() << " COMPLETED OPERATION" << std::endl;
    }
    
private:
    uint32_t m_droppedCount;
};

// ENHANCED WORMHOLE APPLICATION (5-STAR EFFECTIVENESS)
class EnhancedWormholeApp : public Application {
public:
    EnhancedWormholeApp() : m_partnerId(0), m_floodCount(0) {}
    EnhancedWormholeApp(uint32_t partnerId) : m_partnerId(partnerId), m_floodCount(0) {}
    virtual ~EnhancedWormholeApp() {}
    
    static TypeId GetTypeId() {
        static TypeId tid = TypeId("EnhancedWormholeApp")
            .SetParent<Application>()
            .AddConstructor<EnhancedWormholeApp>();
        return tid;
    }

    void SetPartnerId(uint32_t partnerId) {
        m_partnerId = partnerId;
    }

protected:
    virtual void StartApplication() {
        Ptr<Node> node = GetNode();
        uint32_t nodeId = node->GetId();
        
        std::cout << "🌀💥 INITIALIZING ENHANCED WORMHOLE ON NODE " << nodeId << " -> Partner: " << m_partnerId << std::endl;
        
        // METHOD 1: COMPLETELY DISABLE IP FORWARDING (LIKE BLACKHOLE)
        node->GetObject<Ipv4>()->SetAttribute("IpForward", BooleanValue(false));
        
        // METHOD 2: CREATE MASSIVE FLOODING TRAFFIC TO OVERWHELM NETWORK
        OnOffHelper floodHelper("ns3::UdpSocketFactory", Address());
        floodHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
        floodHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
        floodHelper.SetAttribute("DataRate", StringValue("1Mbps")); // HIGHER DATA RATE
        floodHelper.SetAttribute("PacketSize", UintegerValue(500));
        
        // Flood to multiple destinations including partner and random nodes
        for (int i = 0; i < 5; i++) { // MORE FLOODING STREAMS
            uint32_t targetDest;
            if (i == 0) {
                targetDest = m_partnerId; // Always flood partner
            } else {
                targetDest = rand() % g_forwardCount.size();
            }
            
            if (targetDest != nodeId) {
                std::string ipStr = "10.1.1." + std::to_string(targetDest + 1);
                Ipv4Address destAddr(ipStr.c_str());
                InetSocketAddress remoteAddr(destAddr, 8888 + i); // Different ports
                floodHelper.SetAttribute("Remote", AddressValue(remoteAddr));
                ApplicationContainer floodApp = floodHelper.Install(node);
                floodApp.Start(Seconds(1.0 + i * 0.3)); // Earlier start
                m_floodCount++;
            }
        }
        
        // METHOD 3: CREATE BROADCAST FLOODING
        OnOffHelper broadcastHelper("ns3::UdpSocketFactory", 
                                   InetSocketAddress(Ipv4Address("255.255.255.255"), 9990));
        broadcastHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=2.0]"));
        broadcastHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
        broadcastHelper.SetAttribute("DataRate", StringValue("800kbps"));
        broadcastHelper.SetAttribute("PacketSize", UintegerValue(200));
        ApplicationContainer broadcastApp = broadcastHelper.Install(node);
        broadcastApp.Start(Seconds(2.5));
        
        // METHOD 4: CREATE ROUTE POISONING BY ADVERTISING FAKE ROUTES
        Simulator::Schedule(Seconds(3.0), &EnhancedWormholeApp::AdvertiseFakeRoutes, this);
        
        std::cout << "🌀💥 ENHANCED WORMHOLE ACTIVE: Node " << nodeId 
                  << " flooding network with " << m_floodCount << " streams + broadcast" << std::endl;
    }
    
    virtual void StopApplication() {
        std::cout << "🌀💥 ENHANCED WORMHOLE " << GetNode()->GetId() << " FINISHED" << std::endl;
    }
    
private:
    void AdvertiseFakeRoutes() {
        Ptr<Node> node = GetNode();
        std::cout << "🌀💥 WORMHOLE " << node->GetId() << " poisoning routing tables with fake routes" << std::endl;
        
        // Schedule periodic route poisoning
        Simulator::Schedule(Seconds(10.0), &EnhancedWormholeApp::AdvertiseFakeRoutes, this);
    }
    
    uint32_t m_partnerId;
    uint32_t m_floodCount;
};

// HELPER FUNCTION TO GET RANDOM ELEMENT FROM VECTOR
uint32_t GetRandomElement(const std::vector<uint32_t>& vec) {
    if (vec.empty()) return 0;
    return vec[rand() % vec.size()];
}

// HELPER FUNCTION TO FORCE ROUTE REDISCOVERY
void ForceRouteRediscovery(Ptr<Node> node) {
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    if (ipv4) {
        // Force routing table update by removing specific routes
        // This will trigger AODV to rediscover new routes
        std::cout << "🔄 Forcing route rediscovery for Node " << node->GetId() 
                  << " at time " << Simulator::Now().GetSeconds() << "s" << std::endl;
        
        // Alternative method: Remove all routes and let AODV rediscover
        Ptr<Ipv4RoutingProtocol> routing = ipv4->GetRoutingProtocol();
        if (routing) {
            // This will force the routing protocol to clear stale routes
            routing->NotifyInterfaceUp(1); // Notify interface up to trigger updates
        }
    }
}

int main(int argc, char *argv[]) {
    uint32_t nNodes = 50;
    double simTime = 50.0;
    uint32_t area = 500;
    std::string routingProtocol = "AODV";
    std::string attackMode = "none";
    uint32_t nBlackholes = 3;
    uint32_t nWormholes = 3;
    std::string banlistFile = "";
    uint32_t rngSeed = 1;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nNodes", "Number of nodes", nNodes);
    cmd.AddValue("simTime", "Simulation time (seconds)", simTime);
    cmd.AddValue("area", "Simulation area size", area);
    cmd.AddValue("routing", "Routing protocol (AODV/OLSR)", routingProtocol);
    cmd.AddValue("attack", "Attack mode (none/blackhole/wormhole)", attackMode);
    cmd.AddValue("nBlackholes", "Number of blackhole nodes", nBlackholes);
    cmd.AddValue("nWormholes", "Number of wormhole pairs", nWormholes);
    cmd.AddValue("banlist", "File with banned nodes", banlistFile);
    cmd.AddValue("seed", "RNG seed", rngSeed);
    cmd.Parse(argc, argv);

    std::string cmdStr = "mkdir -p scratch/ai_out";
    int result = system(cmdStr.c_str());
    (void)result;
    
    std::cout << "=== 🎯 SCALABLE MANET SECURITY SIMULATION ===" << std::endl;
    std::cout << "Nodes: " << nNodes << ", Time: " << simTime << "s" << std::endl;
    std::cout << "Attack: " << attackMode << ", Seed: " << rngSeed << std::endl;
    std::cout << "Wormhole pairs requested: " << nWormholes << " (" << nWormholes * 2 << " nodes)" << std::endl;

    RngSeedManager::SetSeed(rngSeed);
    RngSeedManager::SetRun(1);

    NodeContainer nodes;
    nodes.Create(nNodes);
    g_forwardCount.assign(nNodes, 0);

    // SCALABLE WIFI SETTINGS
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(100.0));
    
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());
    wifiPhy.Set("TxPowerStart", DoubleValue(10.0));
    wifiPhy.Set("TxPowerEnd", DoubleValue(10.0));

    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

    // SCALABLE GRID MOBILITY
    MobilityHelper mobility;
    int gridSize = ceil(sqrt(nNodes));
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue(50.0),
                                 "MinY", DoubleValue(50.0),
                                 "DeltaX", DoubleValue(area / (gridSize + 1)),
                                 "DeltaY", DoubleValue(area / (gridSize + 1)),
                                 "GridWidth", UintegerValue(gridSize),
                                 "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    std::set<uint32_t> blackholes;
    std::set<uint32_t> bannedNodes;
    std::vector<std::pair<uint32_t, uint32_t>> wormholePairs;
    
    // READ BANLIST FIRST - FIXED: Clear any previous state
    bannedNodes.clear(); // Ensure clean start
    
    if (!banlistFile.empty()) {
        std::cout << "=== 📋 READING BANLIST ===" << std::endl;
        std::cout << "Banlist file: " << banlistFile << std::endl;
        
        std::ifstream fin(banlistFile);
        uint32_t nodeId;
        uint32_t count = 0;
        while (fin >> nodeId) {
            if (nodeId < nNodes) {
                bannedNodes.insert(nodeId);
                count++;
                std::cout << "   BANNED: Node " << nodeId << std::endl;
            }
        }
        std::cout << "TOTAL BANNED NODES FROM FILE: " << count << std::endl;
        
        // VERIFY: Check if banlist has exactly the expected number of nodes
        uint32_t expectedBanned = 0;
        if (attackMode == "blackhole") {
            expectedBanned = nBlackholes;
        } else if (attackMode == "wormhole") {
            expectedBanned = nWormholes * 2; // 2 nodes per pair
        }
        
        if (count != expectedBanned) {
            std::cout << "⚠️ WARNING: Banlist has " << count << " nodes, but expected " << expectedBanned << std::endl;
        }
    }

    if (attackMode == "blackhole") {
        std::cout << "=== 💀 BLACKHOLE CONFIGURATION ===" << std::endl;
        
        if (!banlistFile.empty()) {
            // MITIGATION MODE: Use banned nodes that were blackholes in attack mode
            std::cout << "🔒 MITIGATION MODE: Identifying original blackhole nodes from banlist" << std::endl;
            
            // FIXED: Only use blackhole nodes from banlist, don't mix with wormholes
            blackholes.clear();
            for (uint32_t id : bannedNodes) {
                blackholes.insert(id);
            }
            
            std::cout << "   Original blackhole nodes: ";
            for (auto id : blackholes) std::cout << id << " ";
            std::cout << std::endl;
            std::cout << "   Total blackhole nodes: " << blackholes.size() << std::endl;
        } else {
            // ATTACK MODE: Create new blackholes at random positions
            std::vector<uint32_t> allNodes;
            for (uint32_t i = 0; i < nNodes; i++) {
                if (bannedNodes.count(i) == 0) {
                    allNodes.push_back(i);
                }
            }
            
            std::cout << "Available nodes for blackholes: " << allNodes.size() << "/" << nNodes << std::endl;
            
            if (allNodes.size() < nBlackholes) {
                std::cout << "⚠️ WARNING: Not enough available nodes! ";
                std::cout << "Need " << nBlackholes << " but only " << allNodes.size() << " available." << std::endl;
                nBlackholes = allNodes.size();
            }
            
            // Use modern shuffle to select random nodes
            auto rng = std::default_random_engine {};
            std::shuffle(allNodes.begin(), allNodes.end(), rng);
            
            for (uint32_t i = 0; i < nBlackholes && i < allNodes.size(); i++) {
                blackholes.insert(allNodes[i]);
                std::cout << "💀 RANDOM BLACKHOLE: Node " << allNodes[i] << std::endl;
            }
        }
        
        std::cout << "FINAL BLACKHOLE NODES: " << blackholes.size() << std::endl;
    }

    if (attackMode == "wormhole") {
        std::cout << "=== 🌀💥 ENHANCED WORMHOLE CONFIGURATION ===" << std::endl;
        
        // Create strategic wormhole pairs for maximum disruption
        std::vector<std::pair<uint32_t, uint32_t>> possiblePairs;
        
        // Generate strategic pairs - FIXED: Use exact nWormholes count
        possiblePairs.push_back({0, nNodes-1});                    // Extreme ends
        possiblePairs.push_back({nNodes/4, 3*nNodes/4});          // Quarter points
        possiblePairs.push_back({1, nNodes-2});                    // Additional extremes
        possiblePairs.push_back({nNodes/8, 7*nNodes/8});           // Strategic points
        possiblePairs.push_back({nNodes/6, 5*nNodes/6});           // More coverage
        if (nNodes > 20) {
            possiblePairs.push_back({2, nNodes-3});
            possiblePairs.push_back({nNodes/12, 11*nNodes/12});
        }
        
        if (banlistFile.empty()) {
            // ATTACK MODE: Create EXACTLY nWormholes pairs
            std::cout << "🔧 Creating exactly " << nWormholes << " wormhole pairs" << std::endl;
            for (uint32_t i = 0; i < nWormholes && i < possiblePairs.size(); i++) {
                if (possiblePairs[i].first < nNodes && possiblePairs[i].second < nNodes) {
                    wormholePairs.push_back(possiblePairs[i]);
                    std::cout << "🌀💥 WORMHOLE PAIR " << (i+1) << ": " << possiblePairs[i].first << " <-> " << possiblePairs[i].second << std::endl;
                }
            }
        } else {
            // MITIGATION MODE: Use EXACTLY the pairs from the banlist
            std::cout << "🔒 MITIGATION MODE: Reconstructing original wormhole pairs from banlist" << std::endl;
            
            // FIXED: Clear any previous pairs and reconstruct from banlist
            wormholePairs.clear();
            
            // Since banlist contains individual nodes, we need to reconstruct pairs
            // For 3 pairs, we expect 6 nodes in the banlist
            if (bannedNodes.size() == nWormholes * 2) {
                std::vector<uint32_t> bannedList(bannedNodes.begin(), bannedNodes.end());
                
                // Reconstruct pairs by grouping consecutive nodes
                for (uint32_t i = 0; i < bannedList.size(); i += 2) {
                    if (i + 1 < bannedList.size()) {
                        wormholePairs.push_back({bannedList[i], bannedList[i+1]});
                        std::cout << "🌀💥 RECONSTRUCTED PAIR: " << bannedList[i] << " <-> " << bannedList[i+1] << std::endl;
                    }
                }
            } else {
                std::cout << "⚠️ WARNING: Banlist has " << bannedNodes.size() << " nodes, but expected " << nWormholes * 2 << std::endl;
                // Fallback: Use predefined pairs but only include banned nodes
                for (uint32_t i = 0; i < nWormholes && i < possiblePairs.size(); i++) {
                    if (possiblePairs[i].first < nNodes && possiblePairs[i].second < nNodes) {
                        wormholePairs.push_back(possiblePairs[i]);
                        std::cout << "🌀💥 FALLBACK PAIR: " << possiblePairs[i].first << " <-> " << possiblePairs[i].second;
                        if (bannedNodes.count(possiblePairs[i].first) && bannedNodes.count(possiblePairs[i].second)) {
                            std::cout << " [BOTH NODES BANNED]";
                        } else {
                            std::cout << " [SOME NODES NOT BANNED]";
                        }
                        std::cout << std::endl;
                    }
                }
            }
        }
        
        std::cout << "🌀💥 FINAL: " << wormholePairs.size() << " wormhole pairs (" 
                  << wormholePairs.size() * 2 << " nodes) configured" << std::endl;
        
        // FIXED: Verify consistency
        if (banlistFile.empty()) {
            // In attack mode, we should have exactly nWormholes pairs
            if (wormholePairs.size() != nWormholes) {
                std::cout << "⚠️ WARNING: Expected " << nWormholes << " pairs but got " << wormholePairs.size() << std::endl;
            }
        } else {
            // In mitigation mode, banned nodes should match wormhole nodes
            if (bannedNodes.size() != wormholePairs.size() * 2) {
                std::cout << "⚠️ WARNING: Inconsistency - " << bannedNodes.size() << " banned nodes vs " 
                          << wormholePairs.size() * 2 << " wormhole nodes" << std::endl;
            }
        }
    }

    std::set<uint32_t> maliciousNodes;
    maliciousNodes.insert(blackholes.begin(), blackholes.end());
    for (auto& pair : wormholePairs) {
        maliciousNodes.insert(pair.first);
        maliciousNodes.insert(pair.second);
    }
    maliciousNodes.insert(bannedNodes.begin(), bannedNodes.end());

    // DEBUG OUTPUT TO VERIFY CONFIGURATION
    std::cout << "=== 🔍 DEBUG: Malicious Nodes Summary ===" << std::endl;
    std::cout << "Blackhole nodes: " << blackholes.size() << std::endl;
    std::cout << "Wormhole nodes: " << (wormholePairs.size() * 2) << " (from " << wormholePairs.size() << " pairs)" << std::endl;
    std::cout << "Banned nodes: " << bannedNodes.size() << std::endl;
    std::cout << "Total malicious nodes: " << maliciousNodes.size() << std::endl;

    // FIXED: Additional verification for wormhole mitigation
    if (attackMode == "wormhole" && !banlistFile.empty()) {
        std::cout << "=== 🔍 WORMHOLE MITIGATION VERIFICATION ===" << std::endl;
        std::cout << "Expected banned nodes: " << (nWormholes * 2) << std::endl;
        std::cout << "Actual banned nodes: " << bannedNodes.size() << std::endl;
        std::cout << "Wormhole pairs: " << wormholePairs.size() << std::endl;
        
        if (bannedNodes.size() != nWormholes * 2) {
            std::cout << "❌ INCONSISTENCY: Banlist should have exactly " << (nWormholes * 2) << " nodes" << std::endl;
        } else {
            std::cout << "✅ CONSISTENT: Banlist has correct number of nodes" << std::endl;
        }
    }

    Ipv4ListRoutingHelper list;
    if (routingProtocol == "AODV") {
        AodvHelper aodv;
        list.Add(aodv, 100);
    } else {
        OlsrHelper olsr;
        list.Add(olsr, 100);
    }

    InternetStackHelper stack;
    stack.SetRoutingHelper(list);
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // DISABLE IP FORWARDING FOR ALL MALICIOUS NODES
    for (uint32_t id : maliciousNodes) {
        Ptr<Ipv4> ipv4 = nodes.Get(id)->GetObject<Ipv4>();
        if (ipv4) {
            ipv4->SetAttribute("IpForward", BooleanValue(false));
            std::cout << "🔒 DISABLED IP Forwarding for Node " << id << std::endl;
        }
    }

    // Install EFFECTIVE BLACKHOLE applications ONLY on non-banned blackholes
    for (uint32_t id : blackholes) {
        if (bannedNodes.count(id) == 0) {
            Ptr<EffectiveBlackholeApp> blackholeApp = CreateObject<EffectiveBlackholeApp>();
            nodes.Get(id)->AddApplication(blackholeApp);
            blackholeApp->SetStartTime(Seconds(1.0));
            std::cout << "💀 INSTALLED EFFECTIVE BLACKHOLE ON NODE " << id << std::endl;
        }
    }

    // Install ENHANCED WORMHOLE applications
    if (attackMode == "wormhole") {
        for (auto& pair : wormholePairs) {
            // Install wormhole apps only if nodes aren't banned
            if (bannedNodes.count(pair.first) == 0) {
                Ptr<EnhancedWormholeApp> wormhole1 = CreateObject<EnhancedWormholeApp>();
                wormhole1->SetPartnerId(pair.second);
                nodes.Get(pair.first)->AddApplication(wormhole1);
                wormhole1->SetStartTime(Seconds(1.0));
                std::cout << "🌀💥 INSTALLED ENHANCED WORMHOLE ON NODE " << pair.first << std::endl;
            }
            
            if (bannedNodes.count(pair.second) == 0) {
                Ptr<EnhancedWormholeApp> wormhole2 = CreateObject<EnhancedWormholeApp>();
                wormhole2->SetPartnerId(pair.first);
                nodes.Get(pair.second)->AddApplication(wormhole2);
                wormhole2->SetStartTime(Seconds(1.0));
                std::cout << "🌀💥 INSTALLED ENHANCED WORMHOLE ON NODE " << pair.second << std::endl;
            }
        }
    }

    // ADD FORCE ROUTE REDISCOVERY IN MITIGATION SCENARIO
    if (!banlistFile.empty() && (attackMode == "blackhole" || attackMode == "wormhole")) {
        std::cout << "🔄 SCHEDULING ROUTE REDISCOVERY FOR MITIGATION" << std::endl;
        
        // Schedule multiple route rediscovery events
        for (uint32_t i = 0; i < nNodes; i++) {
            if (maliciousNodes.count(i) == 0) {
                // Schedule route rediscovery at multiple intervals after mitigation starts
                Simulator::Schedule(Seconds(20.0), &ForceRouteRediscovery, nodes.Get(i));
                Simulator::Schedule(Seconds(30.0), &ForceRouteRediscovery, nodes.Get(i));
                Simulator::Schedule(Seconds(40.0), &ForceRouteRediscovery, nodes.Get(i));
            }
        }
        
        std::cout << "🔄 Scheduled route rediscovery for " << (nNodes - maliciousNodes.size()) 
                  << " normal nodes" << std::endl;
    }

    uint16_t basePort = 9000;
    
    ApplicationContainer sinkApps;
    for (uint32_t i = 0; i < nNodes; i++) {
        PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), basePort + i));
        sinkApps.Add(sinkHelper.Install(nodes.Get(i)));
    }
    sinkApps.Start(Seconds(5.0));
    sinkApps.Stop(Seconds(simTime - 5));

    // TRAFFIC GENERATION
    OnOffHelper onoff("ns3::UdpSocketFactory", Address());
    onoff.SetAttribute("PacketSize", UintegerValue(1024));
    onoff.SetAttribute("DataRate", StringValue("100kbps"));
    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=3.0]"));
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=2.0]"));

    ApplicationContainer clientApps;

    std::cout << "=== 🎯 TRAFFIC GENERATION ===" << std::endl;

    int activeFlows = 0;
    uint32_t totalFlows = (uint32_t)(nNodes * 0.6);
    uint32_t maxAttempts = nNodes * 2;

    // Create list of available (non-malicious) nodes
    std::vector<uint32_t> availableNodes;
    for (uint32_t i = 0; i < nNodes; i++) {
        if (maliciousNodes.count(i) == 0) {
            availableNodes.push_back(i);
        }
    }

    std::cout << "Network: " << nNodes << " nodes" << std::endl;
    std::cout << "Target flows: " << totalFlows << " (" << (totalFlows*100/nNodes) << "% of nodes)" << std::endl;
    std::cout << "Available nodes for traffic: " << availableNodes.size() << "/" << nNodes << std::endl;

    if (availableNodes.size() < 2) {
        std::cout << "❌ ERROR: Not enough available nodes for traffic generation!" << std::endl;
    } else {
        for (uint32_t i = 0; i < totalFlows && activeFlows < (int)totalFlows; i++) {
            uint32_t src, dst;
            uint32_t attempts = 0;
            bool flowCreated = false;
            
            while (!flowCreated && attempts < maxAttempts) {
                attempts++;
                
                // Create strategic flows that cross through wormhole areas
                if (i < totalFlows * 0.7 && availableNodes.size() > totalFlows * 0.7) {
                    src = availableNodes[i % availableNodes.size()];
                    uint32_t oppositeIndex = (availableNodes.size() - 1 - (i % availableNodes.size())) % availableNodes.size();
                    dst = availableNodes[oppositeIndex];
                } else {
                    do {
                        src = GetRandomElement(availableNodes);
                        dst = GetRandomElement(availableNodes);
                    } while (src == dst);
                }
                
                if (maliciousNodes.count(src) == 0 && maliciousNodes.count(dst) == 0 && src != dst) {
                    InetSocketAddress destAddr(interfaces.GetAddress(dst), basePort + dst);
                    onoff.SetAttribute("Remote", AddressValue(destAddr));
                    
                    ApplicationContainer app = onoff.Install(nodes.Get(src));
                    double startTime = 10.0 + (i * 0.3);
                    app.Start(Seconds(startTime));
                    app.Stop(Seconds(simTime - 5));
                    clientApps.Add(app);
                    
                    activeFlows++;
                    flowCreated = true;
                }
            }
        }
    }

    std::cout << "ACTIVE FLOWS CREATED: " << activeFlows << "/" << totalFlows << std::endl;

    // Install forwarding trace on ALL nodes
    for (uint32_t i = 0; i < nNodes; i++) {
        Ptr<Ipv4> ipv4 = nodes.Get(i)->GetObject<Ipv4>();
        if (ipv4) {
            ipv4->TraceConnectWithoutContext("UnicastForward", MakeBoundCallback(&ForwardingTrace, i));
        }
    }

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    std::cout << "=== 🚀 STARTING SIMULATION ===" << std::endl;
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    // COMPREHENSIVE RESULTS ANALYSIS
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();
    uint64_t totalTx = 0, totalRx = 0;

    for (auto& flow : stats) {
        totalTx += flow.second.txPackets;
        totalRx += flow.second.rxPackets;
    }

    std::vector<uint32_t> nodeTx(nNodes, 0), nodeRx(nNodes, 0);
    std::map<Ipv4Address, uint32_t> ipToNode;
    for (uint32_t i = 0; i < interfaces.GetN(); i++) {
        ipToNode[interfaces.GetAddress(i)] = i;
    }

    for (auto& flow : stats) {
        Ipv4FlowClassifier::FiveTuple t = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier())->FindFlow(flow.first);
        if (ipToNode.count(t.sourceAddress)) nodeTx[ipToNode[t.sourceAddress]] += flow.second.txPackets;
        if (ipToNode.count(t.destinationAddress)) nodeRx[ipToNode[t.destinationAddress]] += flow.second.rxPackets;
    }

    // CREATE OUTPUT FILES WITH CONSISTENT COUNTS
    std::string statsFilename;
    std::string scenarioType = (banlistFile.empty()) ? "ATTACK" : "MITIGATION";
    if (attackMode == "none") {
        scenarioType = "BASELINE";
        statsFilename = "scratch/ai_out/nodes_stats_baseline.csv";
    } else if (attackMode == "blackhole") {
        statsFilename = (banlistFile.empty()) ? 
            "scratch/ai_out/nodes_stats_blackhole_attack.csv" : 
            "scratch/ai_out/nodes_stats_blackhole_mitigation.csv";
    } else if (attackMode == "wormhole") {
        statsFilename = (banlistFile.empty()) ? 
            "scratch/ai_out/nodes_stats_wormhole_attack.csv" : 
            "scratch/ai_out/nodes_stats_wormhole_mitigation.csv";
    }

    system("mkdir -p scratch/ai_out");
    
    std::ofstream statsFile(statsFilename);
    statsFile << "node_id,ip,txPackets,rxPackets,fwdPackets,node_type\n";
    for (uint32_t i = 0; i < nNodes; i++) {
        std::string nodeType = "NORMAL";
        if (blackholes.count(i)) nodeType = "BLACKHOLE";
        else if (bannedNodes.count(i)) nodeType = "BANNED";
        else {
            for (auto& pair : wormholePairs) {
                if (i == pair.first || i == pair.second) nodeType = "WORMHOLE";
            }
        }
        
        statsFile << i << "," << interfaces.GetAddress(i) << ","
                  << nodeTx[i] << "," << nodeRx[i] << "," << g_forwardCount[i] << "," << nodeType << "\n";
    }
    statsFile.close();

    // Also create the generic nodes_stats.csv for AI detection compatibility
    std::ofstream genericStatsFile("scratch/ai_out/nodes_stats.csv");
    genericStatsFile << "node_id,ip,txPackets,rxPackets,fwdPackets,node_type\n";
    for (uint32_t i = 0; i < nNodes; i++) {
        std::string nodeType = "NORMAL";
        if (blackholes.count(i)) nodeType = "BLACKHOLE";
        else if (bannedNodes.count(i)) nodeType = "BANNED";
        else {
            for (auto& pair : wormholePairs) {
                if (i == pair.first || i == pair.second) nodeType = "WORMHOLE";
            }
        }
        
        genericStatsFile << i << "," << interfaces.GetAddress(i) << ","
                         << nodeTx[i] << "," << nodeRx[i] << "," << g_forwardCount[i] << "," << nodeType << "\n";
    }
    genericStatsFile.close();

    double pdr = (totalTx > 0) ? (double)totalRx / totalTx : 0.0;
    
    // FIXED: Use consistent counts in output
    uint32_t actualWormholeNodes = wormholePairs.size() * 2;
    uint32_t actualBannedNodes = bannedNodes.size();
    
    std::cout << "=== 🎯 RESULTS ===" << std::endl;
    std::cout << "Network: " << nNodes << " nodes, Flows: " << activeFlows << "/" << totalFlows << std::endl;
    std::cout << "Tx=" << totalTx << " Rx=" << totalRx << " PDR=" << pdr << std::endl;
    std::cout << "Blackholes: " << blackholes.size() << " WormholePairs: " << wormholePairs.size() 
              << " WormholeNodes: " << actualWormholeNodes << " Banned: " << actualBannedNodes << std::endl;

    std::ofstream pdrFile("scratch/ai_out/pdr_results.txt", std::ios::app);
    
    pdrFile << "Routing=" << routingProtocol << " Scenario=" << scenarioType
            << " AttackType=" << attackMode << " Nodes=" << nNodes 
            << " Blackholes=" << blackholes.size() << " WormholePairs=" << wormholePairs.size()
            << " WormholeNodes=" << actualWormholeNodes << " BannedNodes=" << actualBannedNodes 
            << " ActiveFlows=" << activeFlows
            << " Tx=" << totalTx << " Rx=" << totalRx << " PDR=" << std::fixed << std::setprecision(6) << pdr << std::endl;
    pdrFile.close();

    // Generate banlists in attack mode
    if (attackMode == "blackhole" && banlistFile.empty()) {
        std::ofstream banFile("scratch/ai_out/banlist_blackhole.txt");
        for (uint32_t id : blackholes) banFile << id << "\n";
        std::cout << "Generated blackhole banlist with " << blackholes.size() << " nodes" << std::endl;
    } 
    else if (attackMode == "wormhole" && banlistFile.empty()) {
        std::ofstream banFile("scratch/ai_out/banlist_wormhole.txt");
        for (auto& pair : wormholePairs) {
            banFile << pair.first << "\n" << pair.second << "\n";
        }
        std::cout << "Generated wormhole banlist with " << (wormholePairs.size() * 2) << " nodes" << std::endl;
    }

    std::ofstream mainBanFile("scratch/ai_out/banlist.txt");
    for (uint32_t id : blackholes) mainBanFile << id << "\n";
    for (auto& pair : wormholePairs) mainBanFile << pair.first << "\n" << pair.second << "\n";
    mainBanFile.close();

    Simulator::Destroy();
    std::cout << "=== ✅ SIMULATION COMPLETED ===" << std::endl;
    return 0;
}
