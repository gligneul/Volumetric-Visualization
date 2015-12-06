set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 1.5   # --- blue
set style line 2 lc rgb '#dd181f' lt 1 lw 2 pt 5 ps 1.5   # --- red
set xlabel 'Tamanho do Passo (h)'
plot 'time.dat' t 'Tempo (Segundos)' w linespoints ls 1, \
     'error.dat' t 'Erro Médio* (%)' w linespoints ls 2
pause -1
