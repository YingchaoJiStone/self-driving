# Dockerfile for plotting with gnuplot

FROM alpine:latest

RUN apk --no-cache add gnuplot python3

COPY Steering.csv .

RUN mkdir /out

RUN gnuplot -e 'set datafile separator comma; set xlabel "Microseconds Elapsed"; set ylabel "Steering"; set title "Ground Steering Request"; set terminal svg enhanced; set output "out/plot.svg"; plot "Steering.csv" using 5:2 with lines lc rgb "blue" title "Our", "" using 5:3 with lines lc rgb "red" title "Oracle"'

RUN \
    count=-1 && \
    sum=0 && \
    while IFS=, read -r col0 col1 col2 col3 col4; do \
    if [ "$count" == "-1" ]; then \
    count=`expr $count + 1` ; \
    else \
    if [ "$col2" != "-0" ]; then \
    count=`expr $count + 1` ; \
    sum=`expr $sum + $col3` ; \
    echo "$col0,$col1,$col2,$col3,$col4" ; \
    fi ; \
    fi ; \
    done < "Steering.csv" && \
    score=$(echo "scale=4; $sum / $count * 100" | bc | tr '.' ',') && \
    echo "Line count: $count sum: $sum score: $score" && \
    touch "out/$score.txt"