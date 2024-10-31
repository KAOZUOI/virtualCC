/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "include/allreduce_app.h"
#include "include/rdma_app.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <cstdint>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DGX_A100_2n");
// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 RDMA    |    |    ns-3 RDMA    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                DataRate, Delay
// ===========================================================================
//
const uint32_t NVLINK_BW = 300; // GB/s
const uint32_t A100_IB_BW = 25; // GB/s
// TODO: Values probably need to be adjusted
const uint32_t NVLINK_DELAY = 1; // ms

void CreateNvlinkTopology(NodeContainer& nodes) {
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(std::to_string(NVLINK_BW) + "Gbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue(std::to_string(NVLINK_DELAY) + "ms"));

    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        for (uint32_t j = 0; j < 8; ++j) {
            // TODO: Create NVLINK topology within a node
            NetDeviceContainer devices = pointToPoint.Install(nodes.Get(i), nodes.Get(i));
        }
    }
}

int main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    NodeContainer nodes;
    nodes.Create(2);

    // TODO : Link Attribute for RDMA
    PointToPointHelper pointToPoint;
    // pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    // pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // TODO: set net device attribute for RDMA
    // Ptr<RdmaApp> rdmaApp = CreateObject<RdmaApp>();

    // Ip addresses assignment
    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // TODO: Allreduce
    // Ptr<AllreduceApp> app = CreateObject<AllreduceApp>();


    NS_LOG_UNCOND("Run Simulation.");

    Simulator::Stop(Seconds(2));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
