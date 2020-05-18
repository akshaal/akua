set terminal png size 1920,1080

set logscale y 2

set output 'train-co2-log-04.png'
plot "train-co2-log-04.dat" u 1:3 smooth acsplines title 'Validation' with lines lw 3, \
     "train-co2-log-04.dat" u 1:2 smooth acsplines title 'Training' with lines lw 3, \
     "train-co2-log-04.dat" u 1:($2+$3) smooth acsplines title 'Total' with lines lw 3
