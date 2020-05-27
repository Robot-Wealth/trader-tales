dplyr for traders
================

We’re going to show how a quant trader can manipulate stock price data
using the dplyr package.

``` r
if (!require("pacman")) install.packages("pacman")
```

    ## Loading required package: pacman

    ## Warning: package 'pacman' was built under R version 3.6.3

``` r
pacman::p_load(tidyverse, here)
```

First, load some price data.

`energystockprices.RDS` contains a data frame of daily price
observations for 3 energy stocks.

``` r
prices <- readRDS(here::here('data','energystockprices.RDS'))
prices
```

    ## # A tibble: 13,314 x 7
    ##    ticker date        open  high   low close   volume
    ##    <chr>  <date>     <dbl> <dbl> <dbl> <dbl>    <dbl>
    ##  1 CVX    1997-12-31  18.3  18.5  18.0  18.0  2807400
    ##  2 XOM    1997-12-31  17.1  17.2  16.9  17.0  6946600
    ##  3 CVX    1998-01-02  17.9  18.3  17.9  18.2  2828000
    ##  4 XOM    1998-01-02  17.0  17.3  16.9  17.2  5657200
    ##  5 CVX    1998-01-05  18.3  18.3  17.6  17.7  4309200
    ##  6 XOM    1998-01-05  17.2  17.3  16.8  17.0  8728400
    ##  7 CVX    1998-01-06  17.6  17.6  17.0  17.3  6251200
    ##  8 XOM    1998-01-06  16.7  16.8  16.3  16.4  9009200
    ##  9 CVX    1998-01-07  17.3  17.9  17.2  17.9  4541800
    ## 10 XOM    1998-01-07  16.4  16.9  16.4  16.9 11663600
    ## # ... with 13,304 more rows

We’ve organised our data so that

  - Every column is variable.
  - Every row is an observation.

In this data set:

  - We have 13,314 rows in our data frame.
  - Each row represents a daily price *observation* for a given stock.
  - For each observation measure the open, high, low and close prices,
    and the volume traded.

This is a very helpful way to stucture your price data. We’ll see how we
can use the dplyr package to manipulate price data for quant analysis.

# The main dplyr verbs

There are 6 main functions to master in `dplyr`.

  - `filter()` picks outs observations (rows) by some filter criteria
  - `arrange()` reorders the observations (rows)
  - `select()` picks out the variables (columns)
  - `mutate()` creates new variables (columns) by applying
    transformations to existing variables
  - `summarise()` allows you to group and summarise data - reducing the
    data into a grouped summary with fewer rows.

Finally, the `group_by()` causes the verbs above to act on a group at a
time, rather than the whole dataset.

We’ll go through them one by one.

## Filter

### Filtering rows for a single stock with `filter()` and ==

``` r
prices %>%
  filter(ticker == 'XOM')
```

    ## # A tibble: 5,538 x 7
    ##    ticker date        open  high   low close   volume
    ##    <chr>  <date>     <dbl> <dbl> <dbl> <dbl>    <dbl>
    ##  1 XOM    1997-12-31  17.1  17.2  16.9  17.0  6946600
    ##  2 XOM    1998-01-02  17.0  17.3  16.9  17.2  5657200
    ##  3 XOM    1998-01-05  17.2  17.3  16.8  17.0  8728400
    ##  4 XOM    1998-01-06  16.7  16.8  16.3  16.4  9009200
    ##  5 XOM    1998-01-07  16.4  16.9  16.4  16.9 11663600
    ##  6 XOM    1998-01-08  16.9  17.0  16.5  16.6  6357600
    ##  7 XOM    1998-01-09  16.6  16.6  16.0  16.2  8060600
    ##  8 XOM    1998-01-12  15.8  16.4  15.7  16.3  8362400
    ##  9 XOM    1998-01-13  16.5  16.7  16.4  16.7  8917000
    ## 10 XOM    1998-01-14  16.7  17.0  16.6  16.9  6375000
    ## # ... with 5,528 more rows

`filter()` returns all observations (rows) which satisfy the criteria
within the `filter()` function.

### Filtering rows for multiple stocks with `filter()` and %in%

``` r
prices %>%
  filter(ticker %in% c('XOM','CVX'))
```

    ## # A tibble: 11,076 x 7
    ##    ticker date        open  high   low close   volume
    ##    <chr>  <date>     <dbl> <dbl> <dbl> <dbl>    <dbl>
    ##  1 CVX    1997-12-31  18.3  18.5  18.0  18.0  2807400
    ##  2 XOM    1997-12-31  17.1  17.2  16.9  17.0  6946600
    ##  3 CVX    1998-01-02  17.9  18.3  17.9  18.2  2828000
    ##  4 XOM    1998-01-02  17.0  17.3  16.9  17.2  5657200
    ##  5 CVX    1998-01-05  18.3  18.3  17.6  17.7  4309200
    ##  6 XOM    1998-01-05  17.2  17.3  16.8  17.0  8728400
    ##  7 CVX    1998-01-06  17.6  17.6  17.0  17.3  6251200
    ##  8 XOM    1998-01-06  16.7  16.8  16.3  16.4  9009200
    ##  9 CVX    1998-01-07  17.3  17.9  17.2  17.9  4541800
    ## 10 XOM    1998-01-07  16.4  16.9  16.4  16.9 11663600
    ## # ... with 11,066 more rows

## Arrange

`arrange()` reorders the rows in your data by one or more criteria.

## Reordering rows in order of volume traded ascending with `arrange()`

``` r
prices %>%
  arrange(desc(volume))
```

    ## # A tibble: 13,314 x 7
    ##    ticker date        open  high   low close    volume
    ##    <chr>  <date>     <dbl> <dbl> <dbl> <dbl>     <dbl>
    ##  1 KMI    2014-11-26  33.5  34.8  33.4  34.7 251563883
    ##  2 KMI    2015-12-09  13.9  15.0  13.8  14.6 196318067
    ##  3 KMI    2015-12-08  13.2  14.0  13.1  13.6 158874323
    ##  4 KMI    2015-12-04  16.4  16.5  14.4  14.6 153007861
    ##  5 KMI    2014-08-11  34.4  34.5  30.5  31.9 139982500
    ##  6 KMI    2015-12-07  13.6  14.4  13.1  14.2 137140540
    ##  7 XOM    2010-06-25  44.1  44.2  43.3  43.3 118023500
    ##  8 XOM    2008-10-10  44.9  46.5  39.7  43.9 112867200
    ##  9 XOM    2009-12-14  51.2  51.6  50.1  50.4  91458900
    ## 10 KMI    2015-12-11  14.3  14.8  13.9  14.4  85326140
    ## # ... with 13,304 more rows

## Reordering rows in order of date descending, then volume ascending with `arrange()`

``` r
prices %>%
  arrange(desc(date), volume)
```

    ## # A tibble: 13,314 x 7
    ##    ticker date        open  high   low close   volume
    ##    <chr>  <date>     <dbl> <dbl> <dbl> <dbl>    <dbl>
    ##  1 CVX    2020-01-03 122.  123.  121.  121.   5458066
    ##  2 KMI    2020-01-03  21.1  21.2  21.0  21.2  9579367
    ##  3 XOM    2020-01-03  71.3  71.4  70.2  70.3 16348846
    ##  4 CVX    2020-01-02 121.  122.  121.  121.   5167733
    ##  5 KMI    2020-01-02  21.2  21.2  21.0  21.0 10209968
    ##  6 XOM    2020-01-02  70.2  71.0  70.2  70.9 12273477
    ##  7 CVX    2019-12-31 120.  121.  119.  121.   4159472
    ##  8 KMI    2019-12-31  21    21.2  21.0  21.2  9842676
    ##  9 XOM    2019-12-31  69.0  69.8  69.0  69.8 13023076
    ## 10 CVX    2019-12-30 120.  121.  120.  120.   4196713
    ## # ... with 13,304 more rows

# Select

## Selecting columns with `select()`

Here we use `select()` to pull out the ticker, date, close columns.

``` r
prices %>%
  select(ticker, date, close)
```

    ## # A tibble: 13,314 x 3
    ##    ticker date       close
    ##    <chr>  <date>     <dbl>
    ##  1 CVX    1997-12-31  18.0
    ##  2 XOM    1997-12-31  17.0
    ##  3 CVX    1998-01-02  18.2
    ##  4 XOM    1998-01-02  17.2
    ##  5 CVX    1998-01-05  17.7
    ##  6 XOM    1998-01-05  17.0
    ##  7 CVX    1998-01-06  17.3
    ##  8 XOM    1998-01-06  16.4
    ##  9 CVX    1998-01-07  17.9
    ## 10 XOM    1998-01-07  16.9
    ## # ... with 13,304 more rows

## Chaining transformations together: Filtering, electing and reordering.

Now we’ll see how we can chain transformations together. We’re going to
filter out a single stock, then select the date and close price, then
reorder by date descending.

``` r
prices %>%
  filter(ticker == 'XOM') %>%
  select(date, close) %>%
  arrange(desc(date))
```

    ## # A tibble: 5,538 x 2
    ##    date       close
    ##    <date>     <dbl>
    ##  1 2020-01-03  70.3
    ##  2 2020-01-02  70.9
    ##  3 2019-12-31  69.8
    ##  4 2019-12-30  69.5
    ##  5 2019-12-27  69.9
    ##  6 2019-12-26  70.1
    ##  7 2019-12-24  70.0
    ##  8 2019-12-23  70.3
    ##  9 2019-12-20  69.9
    ## 10 2019-12-19  69.4
    ## # ... with 5,528 more rows

When debugging a `dplyr` pipeline, it can be helpful to select each
transformation one by one, and press CTRL-Enter in Rstudio to just run
the selected transformation.

# Mutate

Now we get onto slightly more exciting stuff.

`mutate()` lets us add variables (columns) to our data. These are
usually derived from existing columns.

## Add new variable (column) for log volume with `mutate()`

Here we use mutate to add a column called `logvolume` to our data. It is
calculated as the log of the `volume` variable.

``` r
prices %>%
  mutate(logvolume = log(volume))
```

    ## # A tibble: 13,314 x 8
    ##    ticker date        open  high   low close   volume logvolume
    ##    <chr>  <date>     <dbl> <dbl> <dbl> <dbl>    <dbl>     <dbl>
    ##  1 CVX    1997-12-31  18.3  18.5  18.0  18.0  2807400      14.8
    ##  2 XOM    1997-12-31  17.1  17.2  16.9  17.0  6946600      15.8
    ##  3 CVX    1998-01-02  17.9  18.3  17.9  18.2  2828000      14.9
    ##  4 XOM    1998-01-02  17.0  17.3  16.9  17.2  5657200      15.5
    ##  5 CVX    1998-01-05  18.3  18.3  17.6  17.7  4309200      15.3
    ##  6 XOM    1998-01-05  17.2  17.3  16.8  17.0  8728400      16.0
    ##  7 CVX    1998-01-06  17.6  17.6  17.0  17.3  6251200      15.6
    ##  8 XOM    1998-01-06  16.7  16.8  16.3  16.4  9009200      16.0
    ##  9 CVX    1998-01-07  17.3  17.9  17.2  17.9  4541800      15.3
    ## 10 XOM    1998-01-07  16.4  16.9  16.4  16.9 11663600      16.3
    ## # ... with 13,304 more rows

## Add new variable (column) for intraday returns with `mutate()`

Now we add a column for the intraday (open to close) simple returns of
the stock each day.

``` r
prices %>%
  mutate(intradayreturns = close / open - 1)
```

    ## # A tibble: 13,314 x 8
    ##    ticker date        open  high   low close   volume intradayreturns
    ##    <chr>  <date>     <dbl> <dbl> <dbl> <dbl>    <dbl>           <dbl>
    ##  1 CVX    1997-12-31  18.3  18.5  18.0  18.0  2807400        -0.0128 
    ##  2 XOM    1997-12-31  17.1  17.2  16.9  17.0  6946600        -0.00714
    ##  3 CVX    1998-01-02  17.9  18.3  17.9  18.2  2828000         0.0188 
    ##  4 XOM    1998-01-02  17.0  17.3  16.9  17.2  5657200         0.0134 
    ##  5 CVX    1998-01-05  18.3  18.3  17.6  17.7  4309200        -0.0296 
    ##  6 XOM    1998-01-05  17.2  17.3  16.8  17.0  8728400        -0.0131 
    ##  7 CVX    1998-01-06  17.6  17.6  17.0  17.3  6251200        -0.0167 
    ##  8 XOM    1998-01-06  16.7  16.8  16.3  16.4  9009200        -0.0188 
    ##  9 CVX    1998-01-07  17.3  17.9  17.2  17.9  4541800         0.0338 
    ## 10 XOM    1998-01-07  16.4  16.9  16.4  16.9 11663600         0.0308 
    ## # ... with 13,304 more rows

# Summarise

`Summarise()` summarises all our observations to a single row.

## Calculate the mean volume traded for all observations using `summarise()`

Let’s use it to calculate the mean volume traded over our data set.

``` r
prices %>%
  summarise(meanvolume = mean(volume),
            count = n())
```

    ## # A tibble: 1 x 2
    ##   meanvolume count
    ##        <dbl> <int>
    ## 1  11552459. 13314

## Use `summarise()` with `group_by()` to calculate mean traded volume for each stock

That’s not always very useful.

Usually you want to group by a variable, and then summarise that grouped
data.

The `group_by()` function tells the dplyr verbs to operate on each group
one at a time.

If we group by ticker, then call summarise, then `dplyr` will preform
the summary calculations separately for each ticker. We will get a row
for each ticker.

``` r
prices %>% 
  group_by(ticker) %>%
  summarise(meanvolume = mean(volume))
```

    ## # A tibble: 3 x 2
    ##   ticker meanvolume
    ##   <chr>       <dbl>
    ## 1 CVX      7302520.
    ## 2 KMI     11342601.
    ## 3 XOM     15887205.

## Use `summarise()` with multiple `group_by` variables, to calculate the mean traded volume for each stock for each year

``` r
library(lubridate)
```

    ## Warning: package 'lubridate' was built under R version 3.6.3

    ## 
    ## Attaching package: 'lubridate'

    ## The following objects are masked from 'package:dplyr':
    ## 
    ##     intersect, setdiff, union

    ## The following objects are masked from 'package:base':
    ## 
    ##     date, intersect, setdiff, union

``` r
prices %>%
  mutate(year = year(date)) %>%
  group_by(ticker, year) %>%
  summarise(meanvolume = mean(volume),
            obscount = n())
```

    ## # A tibble: 58 x 4
    ## # Groups:   ticker [3]
    ##    ticker  year meanvolume obscount
    ##    <chr>  <dbl>      <dbl>    <int>
    ##  1 CVX     1997   2807400         1
    ##  2 CVX     1998   2938243.      252
    ##  3 CVX     1999   2722284.      252
    ##  4 CVX     2000   3646148.      252
    ##  5 CVX     2001   5385545.      248
    ##  6 CVX     2002   5805867.      252
    ##  7 CVX     2003   5819894.      252
    ##  8 CVX     2004   5796291.      252
    ##  9 CVX     2005   8262745.      252
    ## 10 CVX     2006   9136247.      251
    ## # ... with 48 more rows

# `group_by` with `mutate()`

We can also use `group_by` with `mutate()` to calculate new variables
which are calculated separately for a given variable (or set of
variables)

One way you’ll use this nearly everytime you do any quant analysis is to
calculate periodic returns.

## Using `group_by` with `mutate()` and `lag()` to calculate daily close-to-close returns

``` r
prices %>%
  group_by(ticker) %>%
  arrange(date) %>%
  mutate(c2creturns = close / lag(close) - 1)
```

    ## # A tibble: 13,314 x 8
    ## # Groups:   ticker [3]
    ##    ticker date        open  high   low close   volume c2creturns
    ##    <chr>  <date>     <dbl> <dbl> <dbl> <dbl>    <dbl>      <dbl>
    ##  1 CVX    1997-12-31  18.3  18.5  18.0  18.0  2807400    NA     
    ##  2 XOM    1997-12-31  17.1  17.2  16.9  17.0  6946600    NA     
    ##  3 CVX    1998-01-02  17.9  18.3  17.9  18.2  2828000     0.0122
    ##  4 XOM    1998-01-02  17.0  17.3  16.9  17.2  5657200     0.0113
    ##  5 CVX    1998-01-05  18.3  18.3  17.6  17.7  4309200    -0.0289
    ##  6 XOM    1998-01-05  17.2  17.3  16.8  17.0  8728400    -0.0112
    ##  7 CVX    1998-01-06  17.6  17.6  17.0  17.3  6251200    -0.0256
    ##  8 XOM    1998-01-06  16.7  16.8  16.3  16.4  9009200    -0.0358
    ##  9 CVX    1998-01-07  17.3  17.9  17.2  17.9  4541800     0.0357
    ## 10 XOM    1998-01-07  16.4  16.9  16.4  16.9 11663600     0.0319
    ## # ... with 13,304 more rows

# Summary

Arrange your data so:

  - Every column is variable
  - Every row is an observation

You can then easily use `dplyr` to manipulate that data very
efficiently.

There are 6 main functions to master in `dplyr`.

  - `filter()` picks outs observations (rows) by some filter criteria
  - `arrange()` reorders the observations (rows)
  - `select()` picks out the variables (columns)
  - `mutate()` creates new variables (columns) by applying
    transformations to existing variables
  - `summarise()` allows you to group and summarise data - reducing the
    data into a grouped summary with fewer rows.

The `group_by()` causes the verbs above to act on a group at a time,
rather than the whole dataset.
