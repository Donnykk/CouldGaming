# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a temporary script file.
"""

with open('dc_name_list', 'r') as dc_name_file:
    dc_name_list = dc_name_file.readlines()    
    with open('dc_to_pl_rtt.csv', 'w') as dc_to_pl_outfile:       
        one_line = ''
        for i in dc_name_list:
            one_line += ',' + i.strip('\n')
        dc_to_pl_outfile.write(one_line + '\n')
        with open('dc_to_pl_rtt', 'r') as dc_to_pl_infile:
            for line in dc_to_pl_infile:
                str_list = line.split(' ')
                one_line = ''                
                for counter, i in enumerate(str_list):
                    if counter != 10 and counter != 12:
                        one_line += ',' + i
                dc_to_pl_outfile.write(one_line.lstrip(','))
    with open('dc_to_dc_rtt.csv', 'w') as dc_to_dc_outfile:
        one_line = ''
        for i in dc_name_list:
            one_line += ',' + i.strip('\n')
        dc_to_dc_outfile.write(one_line + '\n')
        with open('dc_to_dc_rtt', 'r') as dc_to_dc_infile:
            for i, line in enumerate(dc_to_dc_infile):                
                one_line = dc_name_list[i].strip('\n')
                line = line.split('\t')
                for it in line:
                    one_line += ',' + it
                dc_to_dc_outfile.write(one_line)
    with open('dc_pricing_server.csv', 'w') as dc_pricing_server_outfile:
        with open('dc_price_server', 'r') as dc_pricing_server_infile:
            for i, line in enumerate(dc_pricing_server_infile):
                dc_pricing_server_outfile.write(dc_name_list[i].strip('\n') + ',' + line)
    with open('dc_pricing_bandwidth.csv', 'w') as dc_pricing_bandwidth_outfile:
        with open('dc_price_bandwidth', 'r') as dc_pricing_bandwidth_infile:
            for i, line in enumerate(dc_pricing_bandwidth_infile):
                dc_pricing_bandwidth_outfile.write(dc_name_list[i].strip('\n') + ',' + line)