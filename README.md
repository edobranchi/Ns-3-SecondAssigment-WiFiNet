# Ns-3-SecondAssigment-WiFiNet
Second Assignment for the "Network Applications" exam.

Assignment Description

Construct a network consisting of at least 4 wireless nodes: 2 Access Points (AP) and 2 nodes (STA). If necessary, increase the number of STA nodes.

The two APs should be "close" (spatially) to each other to cause mutual interference. Configure the 2 APs to use 2 WiFi channels that generate mutual interference (e.g., channels 1 and 2 for 802.11b). Refer to this link for more information.

    Associate the STA nodes with one of the two APs, ensuring some are associated with one AP and some with the other.
    [Optional] Connect the two APs via a CSMA or P2P network. In this case, set up the necessary routing rules.
    Generate traffic to or from the STA nodes (at your discretion).

The simulation should be able to use the following channel models alternately:

    YansWiFiChannel
    SpectrumWiFiChannel (both SpectrumChannel and MultiModelSpectrumChannel)

Verify that:

    The simulation results are consistent between the various channel models when using a single AP.
    The simulation results are different between the Yans and Spectrum models when using two APs.
    The simulation results are consistent between the SpectrumChannel and MultiModelSpectrumChannel models when using two APs.
    The execution speed of the simulation in the various cases.
