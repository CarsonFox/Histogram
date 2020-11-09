make

for i in 1 2 4 8
  echo "Using" $i "processes:"
  time mpiexec -n $i ./Histogram 128 0.0 5.0 200000000 > /dev/null
end

# Histogram bins min max data
#mpiexec -n 3 ./Histogram 10 0.0 5.0 100