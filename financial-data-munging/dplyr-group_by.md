Demonstration of dplyr::group\_by
================

The following code demonstrates how `dplyr::group_by` works.

``` r
library(tidyverse)

my_division <- function(x, y) {
  message("I was just called")
  x / y
}

data <- tibble(x = c(-14, 2.55, -24.4, -0.0557, 6.22))

# Called 1 time
data %>%
  mutate(new = my_division(x, 10))
```

    ## # A tibble: 5 x 2
    ##          x      new
    ##      <dbl>    <dbl>
    ## 1 -14      -1.4    
    ## 2   2.55    0.255  
    ## 3 -24.4    -2.44   
    ## 4  -0.0557 -0.00557
    ## 5   6.22    0.622

``` r
gdata <- data %>% group_by(g = c("a", "a", "b", "b", "c"))

# Called 3 times
gdata %>%
  mutate(new = my_division(x, 10))
```

    ## # A tibble: 5 x 3
    ## # Groups:   g [3]
    ##          x g          new
    ##      <dbl> <chr>    <dbl>
    ## 1 -14      a     -1.4    
    ## 2   2.55   a      0.255  
    ## 3 -24.4    b     -2.44   
    ## 4  -0.0557 b     -0.00557
    ## 5   6.22   c      0.622

If the operation is entirely vectorised, the result will be the same
whether the tibble is grouped or not, since elementwise computations are
not affected by the values of other elements. But as soon as summary
operations are involved, the result depends on the grouping structure
because the summaries are computed from group sections instead of whole
columns.

``` r
# Marginal rescaling
data %>%
  mutate(new = x / sd(x))
```

    ## # A tibble: 5 x 2
    ##          x      new
    ##      <dbl>    <dbl>
    ## 1 -14      -1.09   
    ## 2   2.55    0.198  
    ## 3 -24.4    -1.90   
    ## 4  -0.0557 -0.00434
    ## 5   6.22    0.484

``` r
# Conditional rescaling
gdata %>%
  mutate(new = x / sd(x))
```

    ## # A tibble: 5 x 3
    ## # Groups:   g [3]
    ##          x g          new
    ##      <dbl> <chr>    <dbl>
    ## 1 -14      a     -1.20   
    ## 2   2.55   a      0.218  
    ## 3 -24.4    b     -1.42   
    ## 4  -0.0557 b     -0.00324
    ## 5   6.22   c     NA
