Data set collected for "Understanding the Latency Benefits of Multi-Cloud Webservice Deployments".

Latency data files:
    pl_to_ec2_azure_rtt:
        Median latency in milliseconds between PlanetLab nodes and cloud data centers.
        The file is in following format:
            <plnode name> <rtt to data center 1> <rtt to data center 2> ... <rtt to data center 15>
        The indexing of data centers (also in ec2_azure_region_index file):
            1 ec2 us east
            2 ec2 Oregon
            3 ec2 California
            4 ec2 Europe
            5 ec2 Singapore
            6 ec2 Tokyo
            7 ec2 Sao Paulo
            8 azure east Asia
            9 azure east us
            10 azure north central us
            11 azure north europe
            12 azure south central us
            13 azure southeast asia
            14 azure west europe
            15 azure west us
        '*' means there was no successful measurements between the PL node and the data center.

    pl_to_gae_rtt :
        Median latency in milliseconds between PlanetLab nodes and all Google App Engine IPs we discovered.
        The file is in following format:
            <plnode name> <gae ip 1> <gae ip 2> <gae ip 3> ... <gae ip 29>
        The list of GAE ips can be found in gae_ips file. The ip order in this file is the same as it in the gae_ips file.
    
    pl_to_prefix_latency :
        Median latency in milliseconds between PlanetLab nodes and 110K prefixes.
        The file is in following format:
            <pl ip> <prefix> <rtt>


Other files
    chosen_x_pl_hostname_to_ip_mapping: the mapping between PlanetLab node hostname and its ip.
    ec2_azure_region_index: data center name and index mapping used in pl_to_ec2_azure_rtt file.
    gae_ips: list of gae ips used in in pl_to_gae_rtt file.

Additional FAQs:

Q1: A quick question: to compute the RTT from a prefix to a cloud region, should I simply add up the RTT from that prefix to its closest PlanetLab node (i.e., its intermediate node) and the RTT from its intermediate node to that cloud region?

A: Mostly correct, except that what we did was to use PL node as the intermediate node to estimate the latency from prefixes to their closest cloud data center. If you want to estimate latency to further away data centers, we didn't study the error of that. Also, you need to make sure that the estimation error (figure 2 in the paper) is acceptable for your use.

Q2: Another minor question: do all the latency values in pl_to_prefix_latency file represent RTT values?

A: It should be. Let me know if you find some weird cases.