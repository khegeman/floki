## Algorithms

Research level implementations of some SIMD algorithms. 

### Access Aligned Sort

Implementation based on the algorithm presented in this [blog post](http://seven-degrees-of-freedom.blogspot.com/2010/07/question-of-sorts.html)

The original [paper](http://www.dia.eui.upm.es/asignatu/pro_par/articulos/AASort.pdf) is here. 



#### Example Usage

```cpp

std::vector<int32_t> values
    = { 0,    4,    6,    20,   40,   60,   90,   155,
        1188, 2002, 2244, 2296, 3124, 3226, 3334, 4443,
        10,   24,   46,   120,  140,  260,  390,  455,
        2188, 3002, 4244, 5296, 6124, 6226, 6334, 6443 };

floki::sort(begin(values),end(values));

```

## Tested With

Clang 3.4 on Linux

There are some known issues with g++ and Boost SIMD.  See the comments about aliasing on this [issue](https://github.com/MetaScale/nt2/issues/741).  Until this is resolved, it won't compile without setting the following flags.  


```
 -fno-strict-aliasing -DBOOST_SIMD_NO_STRICT_ALIASING

 ```




## Requires

[Boost](http://boost.org)

[Boost SIMD](http://nt2.metascale.fr/doc/html/the_boost_simd_library.html)


## Benchmark


```
Intel(R) Core(TM) i7-4770S CPU @ 3.10GHz

compiled with clang 3.4 and flags -O3 -mavx
```

65536 random elements in std::vector


data type  | std::sort | floki::sort
------------- | ------------- | -------------
int32_t  | 2.99ms | 0.92ms
float    | 3.33ms | 1.10ms
