set terminal png size 1920,1080

set logscale y 2

set output 'train-co2-log-03-train.png'
plot "train-co2-log-e3.dat" u 1:2 smooth acsplines title '1e-3' with lines lw 3, \
     "train-co2-log-e4.dat" u 1:2 smooth acsplines title '1e-4' with lines lw 3, \
     "train-co2-log-5e4.dat" u 1:2 smooth acsplines title '5e-4' with lines lw 3, \
     "train-co2-log-e5.dat" u 1:2 smooth acsplines title '1e-5' with lines lw 3, \
     "train-co2-log-e6.dat" u 1:2 smooth acsplines title '1e-6' with lines lw 3

set output 'train-co2-log-03-valid.png'
plot "train-co2-log-e3.dat" u 1:3 smooth acsplines title '1e-3' with lines lw 3, \
     "train-co2-log-e4.dat" u 1:3 smooth acsplines title '1e-4' with lines lw 3, \
     "train-co2-log-5e4.dat" u 1:3 smooth acsplines title '5e-4' with lines lw 3, \
     "train-co2-log-e5.dat" u 1:3 smooth acsplines title '1e-5' with lines lw 3, \
     "train-co2-log-e6.dat" u 1:3 smooth acsplines title '1e-6' with lines lw 3

set output 'train-co2-log-03-total.png'
plot "train-co2-log-e3.dat" u 1:($2+$3) smooth acsplines title '1e-3' with lines lw 3, \
     "train-co2-log-e4.dat" u 1:($2+$3) smooth acsplines title '1e-4' with lines lw 3, \
     "train-co2-log-5e4.dat" u 1:($2+$3) smooth acsplines title '5e-4' with lines lw 3, \
     "train-co2-log-e5.dat" u 1:($2+$3) smooth acsplines title '1e-5' with lines lw 3, \
     "train-co2-log-e6.dat" u 1:($2+$3) smooth acsplines title '1e-6' with lines lw 3
