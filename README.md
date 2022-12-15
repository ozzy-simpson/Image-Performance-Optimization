# Optimizing the performance of image processing algorithms

This project optimizes memory- and computationally-intensive image processing algorithms (specifically, rotation and smoothing). Average speedups of 224.66% and 452.00% were achieved, respectively, using various techniques. Details of the optimizations are described in the report. The specs for this project are similar to [this project](https://www2.seas.gwu.edu/~bhagiweb/cs135/homeworks/project4.pdf).

## Performance Improvements
| Dimension | naive\_rotate | my\_rotate | Speedup | naive\_smooth | my\_smooth | Speedup |
| --------- | ------------- | ---------- | ------- | ------------- | ---------- | ------- |
| 256       |               |            |         | 6799          | 1940       | 350.46% |
| 512       | 1020          | 753        | 135.46% | 30430         | 7879       | 386.22% |
| 1024      | 8149          | 3280       | 248.45% | 155057        | 34212      | 453.22% |
| 2048      | 54719         | 45690      | 119.76% | 834334        | 134984     | 618.10% |
| 4096      | 627518        | 158877     | 394.97% |               |            |         |
|           |               | Average:   | 224.66% |               | Average:   | 452.00% |

## Usage
```console
$ make
$ ./driver
```

(If the `make` command fails, try running `make clean` before.)
