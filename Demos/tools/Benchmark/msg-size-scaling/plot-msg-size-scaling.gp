# Teminal and output
set terminal pdfcairo font ',11' size 4.5,9

fres = "result-msg-size-scaling"
fres_csv = "result-msg-size-scaling.csv"
fres_tcp_csv = "result-msg-size-scaling-tcp.csv"

set output fres.".pdf"
set datafile separator ";"

version=system("head -1 ".fres_csv." | cut -c3-")
numruns=system("sed '3q;d' ".fres_csv." | awk  -F ';' '{print $1}'")
participants=system("sed '3q;d' ".fres_csv." | awk -F ';' '{print $2}'")
msgcount=system("sed '3q;d' ".fres_csv." | awk -F ';' '{print $4}'")
duration=system("sed '3q;d' ".fres_csv." | awk -F ';' '{print $5}'")

# Common layout
set multiplot layout 3,1 title "\n\n\n\n\n\n\n\n\n"

set label 1 "{/:Bold Message size scaling}\n\n ".version."\n--number-simulation-runs ".numruns."\n--simulation-duration ".duration."\n--number-participants ".participants."\n--message-count ".msgcount."\n--message-size <var>"\
at screen 0.13,0.97 left

set auto 
set xl 'Message size (kB)' offset 0,0.3
set key t l

# Data columns
# 1        2             3        4         5         6        7        8            9           10              11       12           13       14
# numruns, participants, msgsize, msgcount, duration, numsent, runtime, runtime_err, throughput, throughput_err, speedup, speedup_err, msgrate, msgrate_err

# Plot 1: Throughput
set yl 'Throughput (MiB/s)'
p fres_csv     u ($3/1000):9  w l lc 1 not, '' u ($3/1000):9:10   w yerr pt 2 ps 0.8 lc 1 t 'Throughput (DomainSockets)',\
  fres_tcp_csv u ($3/1000):9  w l lc 2 not, '' u ($3/1000):9:10   w yerr pt 2 ps 0.8 lc 2 t 'Throughput (TCP)'

unset label

# Plot 2: Message rate
set yl 'Message rate (kilocount/s)'
p fres_csv     u ($3/1000):($13/1000) w l lc 1 not, '' u ($3/1000):($13/1000):($14/1000) w yerr pt 2 ps 0.8 lc 1 t 'Message rate (DomainSockets)',\
  fres_tcp_csv u ($3/1000):($13/1000) w l lc 2 not, '' u ($3/1000):($13/1000):($14/1000) w yerr pt 2 ps 0.8 lc 2 t 'Message rate (TCP)'

# Plot 3: Speedup
set yl 'Speedup (virtual time/runtime)'
set log y
set yr[0.1:]
p fres_csv     u ($3/1000):11 w l lc 1 not, '' u ($3/1000):11:12 w yerr pt 2 ps 0.8 lc 1 t 'Speedup (DomainSockets)',\
  fres_tcp_csv u ($3/1000):11 w l lc 2 not, '' u ($3/1000):11:12 w yerr pt 2 ps 0.8 lc 2 t 'Speedup (TCP)', 1 w l lc 0 dt 2 t 'Realtime'

