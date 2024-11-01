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
const uint32_t IB_DELAY = 1; // ms

void CreateNvlinkTopology(NodeContainer& nodes) {
    PointToPointHelper nvLink;
    nvLink.SetDeviceAttribute("DataRate", StringValue(std::to_string(NVLINK_BW) + "Gbps"));
    nvLink.SetChannelAttribute("Delay", StringValue(std::to_string(NVLINK_DELAY) + "ms"));

    NetDeviceContainer nvSwitch;
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        for (uint32_t j = i+1; j < nodes.GetN(); ++j) {
            // Create NVLINK topology within a node
            // Normally, each GPU connects through nvLink to nvSwitch
            // For simplicity, I connect each GPU to other GPUs through nvLink directly
            // All connects are bidirectional, resulting in a fully connected topo
            // As I called "nvSwitch" here
            NetDeviceContainer link = nvLink.Install(nodes.Get(i), nodes.Get(j));
            nvSwitch.Add(link);
        }
    }
}

NetDeviceContainer ConnectToIbSwitch(NodeContainer& nodes, Ptr<Node> ibSwitchNode) {
    PointToPointHelper ibLink;
    ibLink.SetDeviceAttribute("DataRate", StringValue(std::to_string(A100_IB_BW) + "Gbps"));
    ibLink.SetChannelAttribute("Delay", StringValue(std::to_string(IB_DELAY) + "ms"));

    // No comment here
    // refer to topoGraph/a100_topo.py
    NetDeviceContainer ibSwitchNetDevices;
    NetDeviceContainer mlx5s;
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        NetDeviceContainer link = ibLink.Install(nodes.Get(i), ibSwitchNode);
        mlx5s.Add(link.Get(0));
        ibSwitchNetDevices.Add(link.Get(1));
    }

    for (uint32_t i = 0; i < ibSwitchNetDevices.GetN(); ++i) {
        Ptr<NetDevice> dev = ibSwitchNetDevices.Get(i);
        Ptr<BridgeNetDevice> bridge = ibSwitchNode->GetDevice(0)->GetObject<BridgeNetDevice>();
        bridge->AddBridgePort(dev);
    }
    return mlx5s;
}

// TODO: Assign IP address
void AssignAddressIP(NetDeviceContainer& devices, Ptr<Node> ibSwitchNode, Ipv4AddressHelper& address) {
    for (uint32_t i = 0; i < devices.GetN(); ++i) {
        NetDeviceContainer linkDevices;
        linkDevices.Add(devices.Get(i));
        linkDevices.Add(ibSwitchNode->GetDevice(i));
        Ipv4InterfaceContainer interfaces = address.Assign(linkDevices);
        address.NewNetwork();
    }
}
int main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // For convience, treat each GPU as a node
    // Each node has 8 GPUs
    NodeContainer node0;
    node0.Create(8);
    NodeContainer node1;
    node1.Create(8);

    CreateNvlinkTopology(node0);
    CreateNvlinkTopology(node1);

    // Create a node and install BridgeNetDevice to pretend as IB switch
    Ptr<Node> ibSwitchNode = CreateObject<Node>();
    Ptr<BridgeNetDevice> ibSwitch = CreateObject<BridgeNetDevice>();
    ibSwitchNode->AddDevice(ibSwitch);

    // Connect nodes to IB switch
    NetDeviceContainer node0mlx5s = ConnectToIbSwitch(node0, ibSwitchNode);
    NetDeviceContainer node1mlx5s = ConnectToIbSwitch(node1, ibSwitchNode);

    // Ip addresses assignment
    InternetStackHelper stack;
    stack.Install(node0);
    stack.Install(node1);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");
    AssignAddressIP(node0mlx5s, ibSwitchNode, address);
    AssignAddressIP(node1mlx5s, ibSwitchNode, address);

    // TODO : Link Attribute for RDMA

    // TODO: set net device attribute for RDMA
    // Ptr<RdmaApp> rdmaApp = CreateObject<RdmaApp>();

    // TODO: Allreduce
    // Ptr<AllreduceApp> app = CreateObject<AllreduceApp>();


    NS_LOG_UNCOND("Run Simulation.");

    Simulator::Stop(Seconds(2));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
