set terminal png size 1920,1080

set logscale y 2

set output 'train-co2-log-01-valid.png'
plot "train-co2-log-01-nodropout.dat" u 1:3 smooth acsplines title 'No-Dropout' with lines lw 3, \
     "train-co2-log-01-dropout0.2.dat" u 1:3 smooth acsplines title 'Dropout: 0.2' with lines lw 3, \
     "train-co2-log-01-dropout0.3.dat" u 1:3 smooth acsplines title 'Dropout: 0.3' with lines lw 3, \
     "train-co2-log-01-dropout0.4.dat" u 1:3 smooth acsplines title 'Dropout: 0.4' with lines lw 3, \
     "train-co2-log-01-dropout0.5.dat" u 1:3 smooth acsplines title 'Dropout: 0.5' with lines lw 3, \
     "train-co2-log-01-dropout0.6.dat" u 1:3 smooth acsplines title 'Dropout: 0.6' with lines lw 3
                    
set output 'train-co2-log-01-train.png'
plot "train-co2-log-01-nodropout.dat" using 1:2 smooth acsplines title 'No-Dropout' with lines lw 3, \
     "train-co2-log-01-dropout0.2.dat" using 1:2 smooth acsplines title 'Dropout 0.2' with lines lw 3, \
     "train-co2-log-01-dropout0.3.dat" using 1:2 smooth acsplines title 'Dropout 0.3' with lines lw 3, \
     "train-co2-log-01-dropout0.4.dat" using 1:2 smooth acsplines title 'Dropout 0.4' with lines lw 3, \
     "train-co2-log-01-dropout0.5.dat" using 1:2 smooth acsplines title 'Dropout 0.5' with lines lw 3, \
     "train-co2-log-01-dropout0.6.dat" using 1:2 smooth acsplines title 'Dropout 0.6' with lines lw 3

set output 'train-co2-log-01-total.png'
plot "train-co2-log-01-nodropout.dat" using 1:($2+$3) smooth acsplines title 'No-Dropout' with lines lw 3, \
     "train-co2-log-01-dropout0.2.dat" using 1:($2+$3) smooth acsplines title 'Dropout 0.2' with lines lw 3, \
     "train-co2-log-01-dropout0.3.dat" using 1:($2+$3) smooth acsplines title 'Dropout 0.3' with lines lw 3, \
     "train-co2-log-01-dropout0.4.dat" using 1:($2+$3) smooth acsplines title 'Dropout 0.4' with lines lw 3, \
     "train-co2-log-01-dropout0.5.dat" using 1:($2+$3) smooth acsplines title 'Dropout 0.5' with lines lw 3, \
     "train-co2-log-01-dropout0.6.dat" using 1:($2+$3) smooth acsplines title 'Dropout 0.6' with lines lw 3
