# Orderbook
multi-order type orderbook

reference:
https://developer.mantidproject.org/ProfilingWithValgrind.html

benchmarking results:

Cachegrind:
==15394== I   refs:      12,091,370,810
==15394== I1  misses:       163,216,407
==15394== LLi misses:            28,424
==15394== I1  miss rate:           1.35%
==15394== LLi miss rate:           0.00%
==15394== 
==15394== D   refs:       7,149,793,651  (3,966,497,801 rd   + 3,183,295,850 wr)
==15394== D1  misses:         3,321,093  (    3,038,957 rd   +       282,136 wr)
==15394== LLd misses:           144,432  (       45,412 rd   +        99,020 wr)
==15394== D1  miss rate:            0.0% (          0.1%     +           0.0%  )
==15394== LLd miss rate:            0.0% (          0.0%     +           0.0%  )
==15394== 
==15394== LL refs:          166,537,500  (  166,255,364 rd   +       282,136 wr)
==15394== LL misses:            172,856  (       73,836 rd   +        99,020 wr)
==15394== LL miss rate:             0.0% (          0.0%     +           0.0%  )


Chrono:
time taken for 1150000 operations: 1724551 microseconds
avg time per operation: 1.49961 microsec/op

