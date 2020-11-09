make

for i in 1 2 4 8
  echo "Using" $i "processes:"
  time mpiexec -n $i ./Histogram 128 0.0 5.0 268435456 > /dev/null
end

# Histogram bins min max data
#mpiexec -n 2 ./Histogram 10 0.0 5.0 100