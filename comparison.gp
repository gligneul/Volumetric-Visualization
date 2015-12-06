set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 1.5   # --- blue
set style line 2 lc rgb '#dd181f' lt 1 lw 2 pt 5 ps 1.5   # --- red
set xlabel 'Tamanho do Passo (h)'
set terminal png size 800,600 enhanced font "Helvetica,20"
set output 'comparison.png'
plot 'time.dat' t 'Tempo em Segundos' w linespoints ls 1, \
     'error.dat' t 'Erro Médio Quadrático' w linespoints ls 2
pause -1
