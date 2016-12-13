/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NRF24L01pNetwork.h
 * Author: emon1
 *
 * Created on December 13, 2016, 1:18 AM
 */

#ifndef NRF24L01PNETWORK_H
#define NRF24L01PNETWORK_H

#include "NRF24L01p.h"

#include <inttypes.h>
#include <stdint.h>
#include <limits>
#include <stdio.h>
#include <string.h>
#include <cstdlib>

class NRF24L01pNetwork : public NRF24L01p{
public:
    typedef struct network_payload{
        uint16_t fromNodeId;
        uint16_t toNodeId;
        uint8_t pid;
        uint8_t packet_info;
        uint8_t payload[26];
    }network_payload_t;

    uint16_t reachable_Nodes[5];
    
    uint16_t ownNetworkId;
    uint16_t ownNodeId;
    
    
    NRF24L01pNetwork();
    NRF24L01pNetwork(const NRF24L01pNetwork& orig);
    virtual ~NRF24L01pNetwork();
    
    void init_network(uint16_t networkID, uint16_t nodeID);
    void processPacket(Payload_t *payload);
    bool ownIdMatched(Payload_t *payload);
    
    int sendToNetwork(network_payload_t *Netpayload);
    int sendToNodeDirect(network_payload_t *Netpayload);
    int sendToAllAdjacent(network_payload_t *Netpayload);
    bool isNodeReachable(uint16_t NodeId);
    
private:

};

#endif /* NRF24L01PNETWORK_H */
