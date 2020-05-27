How to Deal with Gaps in Large Stock Data Universes
================

When you’re working with [large universes of stock
data](https://robotwealth.com/how-to-get-historical-spx-constituents-data-for-free/)
you’ll come across a lot of challenges:

  - Stocks pay dividends and other distributions that have to be
    accounted for.
  - Stocks are subject to splits and other corporate actions which also
    have to be accounted for.
  - New stocks are listed all the time - you won’t have as much history
    for these stocks as for other stocks.
  - Stocks are delisted, and many datasets do not include the price
    history of delisted stocks
  - Stocks can be suspended or halted for a period of time, leading to
    trading gaps.
  - Companies grow and shrink: the “top 100 stocks by market cap” in
    1990 looks very different to the same group in 2020; “growth stocks”
    in 1990 look very different to “growth stocks” in 2020 etc.

The challenges are well understood, but dealing with them is not always
straightforward.

One significant challenge is gaps in data.

Quant analysis gets very hard if you have missing or misaligned data.

If you’re working with a universe of 1,000 stocks life is a lot easier
if you have an observation for each stock for each trading date,
regardless of whether it actually traded that day. That way:

  - you can always do look-ups by date
  - any grouped aggregations or rolling window aggregations will be
    operating on the date range for every ticker
  - you can easily sense check the size of your data to have
    `trading_days * number_of_stocks` rows.

If you work with “wide” matrix-like data, these challenges are obvious
because you have one row for every date in your data set, and the
columns represent an observation for each ticker.

We usually work with [long or “tidy”
data](https://robotwealth.com/financial-data-manipulation-in-dplyr-for-quant-traders/)
- where each observation is an observation for a stock for a given day.

How do we work productively in this data, whilst still ensuring that we
fill in any gaps in our long data with NAs?

The tidyverse makes this very straightforward. Let me show you\!

First, here’s some dummy data to illustrate the problem:

``` r
library(tidyverse)
```

    ## Warning: package 'tidyverse' was built under R version 3.6.3

    ## -- Attaching packages --------------------------------------------------------------------------------------------------- tidyverse 1.3.0 --

    ## v ggplot2 3.3.0     v purrr   0.3.4
    ## v tibble  3.0.1     v dplyr   0.8.5
    ## v tidyr   1.1.0     v stringr 1.4.0
    ## v readr   1.3.1     v forcats 0.5.0

    ## Warning: package 'ggplot2' was built under R version 3.6.3

    ## Warning: package 'tibble' was built under R version 3.6.3

    ## Warning: package 'tidyr' was built under R version 3.6.3

    ## Warning: package 'purrr' was built under R version 3.6.3

    ## Warning: package 'dplyr' was built under R version 3.6.3

    ## Warning: package 'forcats' was built under R version 3.6.3

    ## -- Conflicts ------------------------------------------------------------------------------------------------------ tidyverse_conflicts() --
    ## x dplyr::filter() masks stats::filter()
    ## x dplyr::lag()    masks stats::lag()

``` r
testdata <- tibble(date = c(1,1,2,2,2,3,3),
                       ticker = c('AMZN','FB','AMZN','FB','TSLA','AMZN','TSLA'),
                       returns = 1:7 / 100)
testdata
```

    ## # A tibble: 7 x 3
    ##    date ticker returns
    ##   <dbl> <chr>    <dbl>
    ## 1     1 AMZN      0.01
    ## 2     1 FB        0.02
    ## 3     2 AMZN      0.03
    ## 4     2 FB        0.04
    ## 5     2 TSLA      0.05
    ## 6     3 AMZN      0.06
    ## 7     3 TSLA      0.07

  - TSLA is missing from date 1 as it only started trading after the
    others
  - FB is missing from date 3 as it was put on trading halt after Citron
    Research hacked into Zuck’s memory banks

Ideally we want a row for every date for every stock - with returns set
to NA in the case where data is missing.

That way we can always look up a price by date. And we can always be
sure that any grouped operations by ticker return the same size data
set.

Turns out that the `tidyr::complete` function is exactly what we’re
looking for. It turns *implicit* missing values - like the returns for
TSLA on date 1 and FB on date 3 - into *explicit* missing values:

``` r
tidydata <- testdata %>%
  complete(date, ticker)

tidydata
```

    ## # A tibble: 9 x 3
    ##    date ticker returns
    ##   <dbl> <chr>    <dbl>
    ## 1     1 AMZN      0.01
    ## 2     1 FB        0.02
    ## 3     1 TSLA     NA   
    ## 4     2 AMZN      0.03
    ## 5     2 FB        0.04
    ## 6     2 TSLA      0.05
    ## 7     3 AMZN      0.06
    ## 8     3 FB       NA   
    ## 9     3 TSLA      0.07

Easy\!

Now we have a row for every date for every stock.

Now we can safely do grouped aggregations by ticker, on the
understanding that the data is the same size for all tickers, and we’ve
removed one large source of potential analysis screw-up…

``` r
tidydata %>%
  group_by(ticker) %>%
  summarise(count = n())
```

    ## # A tibble: 3 x 2
    ##   ticker count
    ##   <chr>  <int>
    ## 1 AMZN       3
    ## 2 FB         3
    ## 3 TSLA       3

### Another approach

There’s also a more verbose way to achieve our aim, and I’m showing it
here because I think it’s useful to see how different functions and
libraries connect and cross-over in the tidyverse (right now I’m
fascinated by the intersection of the `purrr::map` functions and the
`dplyr::summarise_if, _at, _all` functions…but that’s a story for
another time).

The verbose approach is as follows:

  - use `tidyr::pivot_wide` to reshape the data to row per date, with a
    column for each stock
  - use `tidyr::pivot_long` to reshape it back to its longer format.

Let’s do it step by step…

First, we make it wide:

``` r
widedata <- testdata %>%
  pivot_wider(id_cols = date, names_from = ticker, values_from = returns)

widedata
```

    ## # A tibble: 3 x 4
    ##    date  AMZN    FB  TSLA
    ##   <dbl> <dbl> <dbl> <dbl>
    ## 1     1  0.01  0.02 NA   
    ## 2     2  0.03  0.04  0.05
    ## 3     3  0.06 NA     0.07

Where we had missing rows, we now have `NA`.

Now we make it long again:

``` r
tidydata <- widedata %>%
  pivot_longer(-date, names_to = 'ticker', values_to =  'returns')

tidydata
```

    ## # A tibble: 9 x 3
    ##    date ticker returns
    ##   <dbl> <chr>    <dbl>
    ## 1     1 AMZN      0.01
    ## 2     1 FB        0.02
    ## 3     1 TSLA     NA   
    ## 4     2 AMZN      0.03
    ## 5     2 FB        0.04
    ## 6     2 TSLA      0.05
    ## 7     3 AMZN      0.06
    ## 8     3 FB       NA   
    ## 9     3 TSLA      0.07

And again we have a row for every date for every stock.

``` r
tidydata %>%
  group_by(ticker) %>%
  summarise(count = n())
```

    ## # A tibble: 3 x 2
    ##   ticker count
    ##   <chr>  <int>
    ## 1 AMZN       3
    ## 2 FB         3
    ## 3 TSLA       3

Here’s the complete pipeline:

``` r
testdata %>%
  pivot_wider(id_cols = date, names_from = ticker, values_from = returns) %>%
  pivot_longer(-date, names_to = 'ticker', values_to =  'returns')
```

    ## # A tibble: 9 x 3
    ##    date ticker returns
    ##   <dbl> <chr>    <dbl>
    ## 1     1 AMZN      0.01
    ## 2     1 FB        0.02
    ## 3     1 TSLA     NA   
    ## 4     2 AMZN      0.03
    ## 5     2 FB        0.04
    ## 6     2 TSLA      0.05
    ## 7     3 AMZN      0.06
    ## 8     3 FB       NA   
    ## 9     3 TSLA      0.07

## What if we have more than one variable in our orignal data?

One of the benefits of working with [longer “tidy”
data](https://robotwealth.com/financial-data-manipulation-in-dplyr-for-quant-traders/)
is that we can have multiple variables per date/stock observation.

``` r
testwider <- testdata %>%
  mutate(volume = 100:106,
         otherfeature = 200:206)

testwider
```

    ## # A tibble: 7 x 5
    ##    date ticker returns volume otherfeature
    ##   <dbl> <chr>    <dbl>  <int>        <int>
    ## 1     1 AMZN      0.01    100          200
    ## 2     1 FB        0.02    101          201
    ## 3     2 AMZN      0.03    102          202
    ## 4     2 FB        0.04    103          203
    ## 5     2 TSLA      0.05    104          204
    ## 6     3 AMZN      0.06    105          205
    ## 7     3 TSLA      0.07    106          206

Again, we’re missing data for TSLA on date 1 and FB on date 3, but now
we’re also missing `volume` and `otherfeature` in addition to `returns`.

To use `complete`, nothing changes from earlier:

``` r
testwider %>%
  complete(date, ticker)
```

    ## # A tibble: 9 x 5
    ##    date ticker returns volume otherfeature
    ##   <dbl> <chr>    <dbl>  <int>        <int>
    ## 1     1 AMZN      0.01    100          200
    ## 2     1 FB        0.02    101          201
    ## 3     1 TSLA     NA        NA           NA
    ## 4     2 AMZN      0.03    102          202
    ## 5     2 FB        0.04    103          203
    ## 6     2 TSLA      0.05    104          204
    ## 7     3 AMZN      0.06    105          205
    ## 8     3 FB       NA        NA           NA
    ## 9     3 TSLA      0.07    106          206

However if we want to pivot back and forth, we do the following:

  - use `pivot_wide` to reshape the data to row per date, with a column
    for each stock
  - use `pivot_long` to reshape it back to its longer format
  - use `left_join` to recover the rest of the variables from the
    original data.

<!-- end list -->

``` r
testwider %>%
  pivot_wider(id_cols = date, names_from = ticker, values_from = returns) %>%
  pivot_longer(-date, names_to = 'ticker', values_to =  'returns') %>%
  left_join(testwider, by = c('date', 'ticker', 'returns'))
```

    ## # A tibble: 9 x 5
    ##    date ticker returns volume otherfeature
    ##   <dbl> <chr>    <dbl>  <int>        <int>
    ## 1     1 AMZN      0.01    100          200
    ## 2     1 FB        0.02    101          201
    ## 3     1 TSLA     NA        NA           NA
    ## 4     2 AMZN      0.03    102          202
    ## 5     2 FB        0.04    103          203
    ## 6     2 TSLA      0.05    104          204
    ## 7     3 AMZN      0.06    105          205
    ## 8     3 FB       NA        NA           NA
    ## 9     3 TSLA      0.07    106          206

## Conclusions

  - Missing values in financial data threaten the validity of quant
    analysis due to inadvertent misalignment
  - Wide data tends to highlight such missing data
  - Long data tends to hide it
  - `tidyr::complete` is a succinct and efficient way to ensure that
    missing observations are accounted for with `NA`
  - Like most tasks in R, there is more than one way to go about it. But
    `complete` should be your go-to function.

## Want all the code?

All the code in this post is available in our [github
repo](https://github.com/Robot-Wealth/r-quant-recipes/tree/master/financial-data-munging)
where you can find lots of other recipes and tools to make your life as
a quant researcher easier.

## If you liked this you’ll probably like these too…

  - [Finanical Manipulation in dplyr for Quant
    Traders](https://robotwealth.com/financial-data-manipulation-in-dplyr-for-quant-traders/)
  - [Handling a Large Universe of Stock Price Data in R and Profiling
    with
    profvis](https://robotwealth.com/handling-a-large-universe-of-stock-price-data-in-r-profiling-with-profvis/)
  - [Rolling Mean Correlations in the
    tidyverse](https://robotwealth.com/rolling-mean-correlations-in-the-tidyverse/)
