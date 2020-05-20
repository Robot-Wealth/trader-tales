Tidy JSON Extraction with purrr
================

Here are some tools and recipes for working with JSON data in the
tidyverse.

In particular, we’ll use `purrr::map` functions to extract and transform
our JSON data, and provide intuitive examples of the cross-overs and
differences between `purrr` and `dplyr`.

``` r
library(tidyverse)
```

    ## -- Attaching packages --------------------------------------------------------------------------------- tidyverse 1.3.0 --

    ## v ggplot2 3.3.0     v purrr   0.3.3
    ## v tibble  2.1.3     v dplyr   0.8.3
    ## v tidyr   1.0.0     v stringr 1.4.0
    ## v readr   1.3.1     v forcats 0.4.0

    ## -- Conflicts ------------------------------------------------------------------------------------ tidyverse_conflicts() --
    ## x dplyr::filter() masks stats::filter()
    ## x dplyr::lag()    masks stats::lag()

``` r
library(here)
```

    ## here() starts at C:/Users/Kris/Documents/r-quant-recipes

``` r
library(kableExtra)
```

    ## 
    ## Attaching package: 'kableExtra'

    ## The following object is masked from 'package:dplyr':
    ## 
    ##     group_rows

``` r
pretty_print <- function(df, num_rows) {
  df %>%
  head(num_rows) %>%
    kable() %>%
    kable_styling(full_width = TRUE, position = 'center') %>%
    scroll_box(height = '300px')
}
```

## Load json as nested named lists

This data has been converted from raw JSON to nested named lists using
`jsonlite::fromJSON` with the `simplify` argument set to `FALSE` (that
is, all elements are converted to named lists).

The data consists of market data for SPY options with various strikes
and expiries. We got it from a free trial of
[Orats](https://info.orats.com/dataapi?hsCtaTracking=e95bffda-578d-41f2-93b6-7c2593c664ff%7C64874a9b-3a1d-4a10-b46a-9cf15fcb7543),
whose data API I enjoy almost as much as their orange website.

You can load the data directly from the Orats API with the following
code (just define your API key in the `ORATS_token` variable):

``` r
library(httr)

ORATS_token <- 'YOUR_KEY_HERE'
res <- GET('https://api.orats.io/data/strikes?tickers=SPY', add_headers(Authorization = ORATS_token))

if (http_type(res) == 'application/json') {
  odata <- jsonlite::fromJSON(content(res, 'text'), simplifyVector = FALSE)
} else {
  stop('No json returned')
}

if (http_error(res)) {
  stop(paste('API request error:',status_code(res), odata$message, odata$documentation_url))
} 
```

Now, if you want to read this data directly into a nicely formatted
dataframe, replace the line:

`odata <- jsonlite::fromJSON(content(res, 'text'), simplifyVector =
FALSE)`

with

`odata <- jsonlite::fromJSON(content(res, 'text'), simplifyVector =
TRUE, flatten = TRUE)`

However, you should know that it isn’t always possible to coerce JSON
into nicely shaped dataframes this easily - often the raw JSON won’t
contain primitive types, or will have nested key-value pairs on the same
level as your desired dataframe columns, to name a couple of obstacles.

In that case, it’s useful to have some tools - like the ones in this
post - for wrangling your source data.

## Look inside JSON lists

``` r
str(strikes, max.level = 1)  
```

    ## List of 1
    ##  $ data:List of 2440

This tells us we have a component named “data”. Let’s look at that a
little more closely:

``` r
str(strikes$data, max.level = 1, list.len = 10)
```

    ## List of 2440
    ##  $ :List of 40
    ##  $ :List of 40
    ##  $ :List of 40
    ##  $ :List of 40
    ##  $ :List of 40
    ##  $ :List of 40
    ##  $ :List of 40
    ##  $ :List of 40
    ##  $ :List of 40
    ##  $ :List of 40
    ##   [list output truncated]

This suggests we have homogenous lists of 40 elements each (an
assumption we’ll check shortly).

Let’s look at one of those lists:

``` r
str(strikes$data[[1]])
```

    ## List of 40
    ##  $ ticker          : chr "SPY"
    ##  $ tradeDate       : chr "2020-05-19"
    ##  $ expirDate       : chr "2020-05-29"
    ##  $ dte             : int 11
    ##  $ strike          : int 140
    ##  $ stockPrice      : num 293
    ##  $ callVolume      : int 0
    ##  $ callOpenInterest: int 0
    ##  $ callBidSize     : int 20
    ##  $ callAskSize     : int 23
    ##  $ putVolume       : int 0
    ##  $ putOpenInterest : int 2312
    ##  $ putBidSize      : int 0
    ##  $ putAskSize      : int 7117
    ##  $ callBidPrice    : num 152
    ##  $ callValue       : num 153
    ##  $ callAskPrice    : num 153
    ##  $ putBidPrice     : int 0
    ##  $ putValue        : num 1.12e-25
    ##  $ putAskPrice     : num 0.01
    ##  $ callBidIv       : int 0
    ##  $ callMidIv       : num 0.98
    ##  $ callAskIv       : num 1.96
    ##  $ smvVol          : num 0.476
    ##  $ putBidIv        : int 0
    ##  $ putMidIv        : num 0.709
    ##  $ putAskIv        : num 1.42
    ##  $ residualRate    : num -0.00652
    ##  $ delta           : int 1
    ##  $ gamma           : num 9.45e-16
    ##  $ theta           : num -0.00288
    ##  $ vega            : num 2e-11
    ##  $ rho             : num 0.0384
    ##  $ phi             : num -0.0802
    ##  $ driftlessTheta  : num -6.07e-09
    ##  $ extSmvVol       : num 0.478
    ##  $ extCallValue    : num 153
    ##  $ extPutValue     : num 1.77e-25
    ##  $ spotPrice       : num 293
    ##  $ updatedAt       : chr "2020-05-19 20:02:33"

All these elements look like they can be easily handled. For instance, I
don’t see any more deeply nested lists, weird missing values, or
anything else that looks difficult.

So now I’ll pull out the interesting bit:

``` r
strikes <- strikes[["data"]]
```

## How many observations do we have?

``` r
length(strikes)
```

    ## [1] 2440

## Are all strike sublists identically named?

This is where we’ll check that our sublists are indeed homogeneously
named, as we assumed above:

``` r
strikes %>%
  map(names) %>%  # this applies the base R function names to each sublist, and returns a list of lists with the output
  unique() %>%
  length() == 1
```

    ## [1] TRUE

## Make a dataframe

We should also check the variable types are consistent as we need single
types in each column of a dataframe (although R will warn if it is
forced to coerce one type to another).

Here’s an interesting thing. It uses a nested `purrr::map` to get the
variable types for each element of each sublist. They’re actually not
identical according to this:

``` r
strikes %>%
  map(.f = ~{map_chr(.x, .f = class)}) %>%
  unique() %>%
  length()
```

    ## [1] 39

This is actually a little puzzling. Inspecting the individual objects
suggests that we do have identical types. If anyone has anything to say
about this, I’d love to hear about it in the comments. In any event,
after we make our dataframe, we should check that the variable types are
as expected.

Now, to that dataframe…

`purrr::flatten` removes one level of hierarchy from a list (`unlist`
removes them all). Here, `flatten` is applied to each sub-list in
`strikes` via `purrr::map_df`.

We use the variant `flatten_df` which returns each sublist as a
dataframe, which makes it compatible with `purrr::map_df`,which requires
a function that returns a dataframe.

``` r
strikes_df <- strikes %>%
  map_df(flatten_df)

strikes_df %>%
  pretty_print(30)
```

<div style="border: 1px solid #ddd; padding: 0px; overflow-y: scroll; height:300px; ">

<table class="table" style="margin-left: auto; margin-right: auto;">

<thead>

<tr>

<th style="text-align:left;position: sticky; top:0; background-color: #FFFFFF;">

ticker

</th>

<th style="text-align:left;position: sticky; top:0; background-color: #FFFFFF;">

tradeDate

</th>

<th style="text-align:left;position: sticky; top:0; background-color: #FFFFFF;">

expirDate

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

dte

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

strike

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

stockPrice

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callVolume

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callOpenInterest

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callBidSize

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callAskSize

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putVolume

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putOpenInterest

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putBidSize

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putAskSize

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callBidPrice

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callValue

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callAskPrice

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putBidPrice

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putValue

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putAskPrice

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callBidIv

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callMidIv

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callAskIv

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

smvVol

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putBidIv

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putMidIv

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putAskIv

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

residualRate

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

delta

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

gamma

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

theta

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

vega

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

rho

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

phi

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

driftlessTheta

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

extSmvVol

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

extCallValue

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

extPutValue

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

spotPrice

</th>

<th style="text-align:left;position: sticky; top:0; background-color: #FFFFFF;">

updatedAt

</th>

</tr>

</thead>

<tbody>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

140

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

23

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

2312

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

7117

</td>

<td style="text-align:right;">

152.37

</td>

<td style="text-align:right;">

152.5790

</td>

<td style="text-align:right;">

152.83

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.980149

</td>

<td style="text-align:right;">

1.960300

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.708976

</td>

<td style="text-align:right;">

1.417950

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0028827

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.0383618

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

152.5790

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

145

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

23

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

2322

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

5703

</td>

<td style="text-align:right;">

147.37

</td>

<td style="text-align:right;">

147.5800

</td>

<td style="text-align:right;">

147.83

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.936511

</td>

<td style="text-align:right;">

1.873020

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.676907

</td>

<td style="text-align:right;">

1.353810

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0029856

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.0397319

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

147.5800

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

150

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

1

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

18

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

1912

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

5703

</td>

<td style="text-align:right;">

142.39

</td>

<td style="text-align:right;">

142.5810

</td>

<td style="text-align:right;">

142.83

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.894396

</td>

<td style="text-align:right;">

1.788790

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.645945

</td>

<td style="text-align:right;">

1.291890

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0030886

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.0411020

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

142.5810

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

155

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

22

</td>

<td style="text-align:right;">

23

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

1483

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

5583

</td>

<td style="text-align:right;">

137.36

</td>

<td style="text-align:right;">

137.5820

</td>

<td style="text-align:right;">

137.81

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.844663

</td>

<td style="text-align:right;">

1.689330

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.616016

</td>

<td style="text-align:right;">

1.232030

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0031915

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.0424720

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

137.5820

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

160

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

22

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

929

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

7016

</td>

<td style="text-align:right;">

132.39

</td>

<td style="text-align:right;">

132.5830

</td>

<td style="text-align:right;">

132.85

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.823228

</td>

<td style="text-align:right;">

1.646460

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.587053

</td>

<td style="text-align:right;">

1.174110

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0032945

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.0438421

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

132.5830

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

165

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

22

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

1874

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

5943

</td>

<td style="text-align:right;">

127.39

</td>

<td style="text-align:right;">

127.5840

</td>

<td style="text-align:right;">

127.85

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.784980

</td>

<td style="text-align:right;">

1.569960

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.558997

</td>

<td style="text-align:right;">

1.117990

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0033974

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.0452122

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

127.5840

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

170

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

22

</td>

<td style="text-align:right;">

23

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

4055

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

7407

</td>

<td style="text-align:right;">

122.37

</td>

<td style="text-align:right;">

122.5850

</td>

<td style="text-align:right;">

122.83

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.739266

</td>

<td style="text-align:right;">

1.478530

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.531711

</td>

<td style="text-align:right;">

1.063420

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0035004

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.0465822

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

122.5850

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

175

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

22

</td>

<td style="text-align:right;">

23

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

2992

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

5103

</td>

<td style="text-align:right;">

117.36

</td>

<td style="text-align:right;">

117.5860

</td>

<td style="text-align:right;">

117.81

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.694933

</td>

<td style="text-align:right;">

1.389870

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.504757

</td>

<td style="text-align:right;">

1.009510

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0036034

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.0479523

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

117.5860

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

180

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

31

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

4320

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

9302

</td>

<td style="text-align:right;">

112.39

</td>

<td style="text-align:right;">

112.5870

</td>

<td style="text-align:right;">

112.83

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.668237

</td>

<td style="text-align:right;">

1.336470

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.478572

</td>

<td style="text-align:right;">

0.957144

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0037063

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.0493224

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

112.5870

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

185

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

31

</td>

<td style="text-align:right;">

1200

</td>

<td style="text-align:right;">

5863

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

4686

</td>

<td style="text-align:right;">

107.39

</td>

<td style="text-align:right;">

107.5880

</td>

<td style="text-align:right;">

107.83

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.633658

</td>

<td style="text-align:right;">

1.267320

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.453113

</td>

<td style="text-align:right;">

0.906225

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0038093

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.0506924

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

107.5880

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

190

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

2

</td>

<td style="text-align:right;">

8

</td>

<td style="text-align:right;">

31

</td>

<td style="text-align:right;">

5

</td>

<td style="text-align:right;">

8253

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

4199

</td>

<td style="text-align:right;">

102.39

</td>

<td style="text-align:right;">

102.5890

</td>

<td style="text-align:right;">

102.83

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.600019

</td>

<td style="text-align:right;">

1.200040

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.428340

</td>

<td style="text-align:right;">

0.856680

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0039123

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

0.0520625

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

\-0.0000001

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

102.5890

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

195

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

10

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

31

</td>

<td style="text-align:right;">

130

</td>

<td style="text-align:right;">

5417

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

5965

</td>

<td style="text-align:right;">

97.39

</td>

<td style="text-align:right;">

97.5902

</td>

<td style="text-align:right;">

97.83

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000002

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.567271

</td>

<td style="text-align:right;">

1.134540

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.404219

</td>

<td style="text-align:right;">

0.808438

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000000

</td>

<td style="text-align:right;">

\-0.0040155

</td>

<td style="text-align:right;">

0.0000002

</td>

<td style="text-align:right;">

0.0534326

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

\-0.0000003

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

97.5902

</td>

<td style="text-align:right;">

0.0000003

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

200

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

72

</td>

<td style="text-align:right;">

8

</td>

<td style="text-align:right;">

31

</td>

<td style="text-align:right;">

2139

</td>

<td style="text-align:right;">

11544

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

3657

</td>

<td style="text-align:right;">

92.39

</td>

<td style="text-align:right;">

92.5912

</td>

<td style="text-align:right;">

92.83

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000015

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.535369

</td>

<td style="text-align:right;">

1.070740

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.380715

</td>

<td style="text-align:right;">

0.761431

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

1.000000

</td>

<td style="text-align:right;">

0.0000001

</td>

<td style="text-align:right;">

\-0.0041200

</td>

<td style="text-align:right;">

0.0000008

</td>

<td style="text-align:right;">

0.0548026

</td>

<td style="text-align:right;">

\-0.0801790

</td>

<td style="text-align:right;">

\-0.0000019

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

92.5912

</td>

<td style="text-align:right;">

0.0000016

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

205

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

23

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

2035

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

2949

</td>

<td style="text-align:right;">

87.38

</td>

<td style="text-align:right;">

87.5922

</td>

<td style="text-align:right;">

87.84

</td>

<td style="text-align:right;">

0.00

</td>

<td style="text-align:right;">

0.0000084

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.507409

</td>

<td style="text-align:right;">

1.014820

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.000000

</td>

<td style="text-align:right;">

0.357803

</td>

<td style="text-align:right;">

0.715607

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.999998

</td>

<td style="text-align:right;">

0.0000004

</td>

<td style="text-align:right;">

\-0.0042307

</td>

<td style="text-align:right;">

0.0000037

</td>

<td style="text-align:right;">

0.0561726

</td>

<td style="text-align:right;">

\-0.0801789

</td>

<td style="text-align:right;">

\-0.0000097

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

87.5922

</td>

<td style="text-align:right;">

0.0000092

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

210

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

7

</td>

<td style="text-align:right;">

23

</td>

<td style="text-align:right;">

18

</td>

<td style="text-align:right;">

177

</td>

<td style="text-align:right;">

2745

</td>

<td style="text-align:right;">

2433

</td>

<td style="text-align:right;">

6196

</td>

<td style="text-align:right;">

82.39

</td>

<td style="text-align:right;">

82.5933

</td>

<td style="text-align:right;">

82.83

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0.0000393

</td>

<td style="text-align:right;">

0.02

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.473935

</td>

<td style="text-align:right;">

0.947870

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.670012

</td>

<td style="text-align:right;">

0.691367

</td>

<td style="text-align:right;">

0.712722

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.999992

</td>

<td style="text-align:right;">

0.0000015

</td>

<td style="text-align:right;">

\-0.0043642

</td>

<td style="text-align:right;">

0.0000158

</td>

<td style="text-align:right;">

0.0575421

</td>

<td style="text-align:right;">

\-0.0801784

</td>

<td style="text-align:right;">

\-0.0000402

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

82.5933

</td>

<td style="text-align:right;">

0.0000438

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

215

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

6

</td>

<td style="text-align:right;">

100

</td>

<td style="text-align:right;">

100

</td>

<td style="text-align:right;">

182

</td>

<td style="text-align:right;">

3439

</td>

<td style="text-align:right;">

5133

</td>

<td style="text-align:right;">

4919

</td>

<td style="text-align:right;">

77.40

</td>

<td style="text-align:right;">

77.5945

</td>

<td style="text-align:right;">

77.83

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0.0001671

</td>

<td style="text-align:right;">

0.02

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.444328

</td>

<td style="text-align:right;">

0.888655

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.625396

</td>

<td style="text-align:right;">

0.645686

</td>

<td style="text-align:right;">

0.665975

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.999969

</td>

<td style="text-align:right;">

0.0000056

</td>

<td style="text-align:right;">

\-0.0045768

</td>

<td style="text-align:right;">

0.0000572

</td>

<td style="text-align:right;">

0.0589103

</td>

<td style="text-align:right;">

\-0.0801765

</td>

<td style="text-align:right;">

\-0.0001500

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

77.5945

</td>

<td style="text-align:right;">

0.0001791

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

220

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

7

</td>

<td style="text-align:right;">

23

</td>

<td style="text-align:right;">

31

</td>

<td style="text-align:right;">

253

</td>

<td style="text-align:right;">

67134

</td>

<td style="text-align:right;">

6636

</td>

<td style="text-align:right;">

4014

</td>

<td style="text-align:right;">

72.39

</td>

<td style="text-align:right;">

72.5959

</td>

<td style="text-align:right;">

72.84

</td>

<td style="text-align:right;">

0.01

</td>

<td style="text-align:right;">

0.0006089

</td>

<td style="text-align:right;">

0.02

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.417191

</td>

<td style="text-align:right;">

0.834383

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.581817

</td>

<td style="text-align:right;">

0.600771

</td>

<td style="text-align:right;">

0.619725

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.999894

</td>

<td style="text-align:right;">

0.0000181

</td>

<td style="text-align:right;">

\-0.0050092

</td>

<td style="text-align:right;">

0.0001931

</td>

<td style="text-align:right;">

0.0602742

</td>

<td style="text-align:right;">

\-0.0801705

</td>

<td style="text-align:right;">

\-0.0004800

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

72.5960

</td>

<td style="text-align:right;">

0.0006497

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

225

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

26

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

8

</td>

<td style="text-align:right;">

72

</td>

<td style="text-align:right;">

21021

</td>

<td style="text-align:right;">

4297

</td>

<td style="text-align:right;">

5090

</td>

<td style="text-align:right;">

67.40

</td>

<td style="text-align:right;">

67.5984

</td>

<td style="text-align:right;">

67.84

</td>

<td style="text-align:right;">

0.02

</td>

<td style="text-align:right;">

0.0020239

</td>

<td style="text-align:right;">

0.03

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.388099

</td>

<td style="text-align:right;">

0.776197

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.574524

</td>

<td style="text-align:right;">

0.586509

</td>

<td style="text-align:right;">

0.598494

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.999669

</td>

<td style="text-align:right;">

0.0000522

</td>

<td style="text-align:right;">

\-0.0060175

</td>

<td style="text-align:right;">

0.0005729

</td>

<td style="text-align:right;">

0.0616259

</td>

<td style="text-align:right;">

\-0.0801525

</td>

<td style="text-align:right;">

\-0.0013866

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

67.5985

</td>

<td style="text-align:right;">

0.0021449

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

230

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

1

</td>

<td style="text-align:right;">

3

</td>

<td style="text-align:right;">

22

</td>

<td style="text-align:right;">

20

</td>

<td style="text-align:right;">

496

</td>

<td style="text-align:right;">

60686

</td>

<td style="text-align:right;">

2857

</td>

<td style="text-align:right;">

5356

</td>

<td style="text-align:right;">

62.41

</td>

<td style="text-align:right;">

62.6033

</td>

<td style="text-align:right;">

62.86

</td>

<td style="text-align:right;">

0.03

</td>

<td style="text-align:right;">

0.0058940

</td>

<td style="text-align:right;">

0.04

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.364226

</td>

<td style="text-align:right;">

0.728453

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.552809

</td>

<td style="text-align:right;">

0.561179

</td>

<td style="text-align:right;">

0.569550

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.999099

</td>

<td style="text-align:right;">

0.0001322

</td>

<td style="text-align:right;">

\-0.0082422

</td>

<td style="text-align:right;">

0.0015687

</td>

<td style="text-align:right;">

0.0629492

</td>

<td style="text-align:right;">

\-0.0801068

</td>

<td style="text-align:right;">

\-0.0035119

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

62.6036

</td>

<td style="text-align:right;">

0.0062251

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

235

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

10

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

63

</td>

<td style="text-align:right;">

4115

</td>

<td style="text-align:right;">

3681

</td>

<td style="text-align:right;">

10023

</td>

<td style="text-align:right;">

57.43

</td>

<td style="text-align:right;">

57.6142

</td>

<td style="text-align:right;">

57.81

</td>

<td style="text-align:right;">

0.04

</td>

<td style="text-align:right;">

0.0157637

</td>

<td style="text-align:right;">

0.05

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.324739

</td>

<td style="text-align:right;">

0.649478

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.523494

</td>

<td style="text-align:right;">

0.530320

</td>

<td style="text-align:right;">

0.537145

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.997754

</td>

<td style="text-align:right;">

0.0003047

</td>

<td style="text-align:right;">

\-0.0129200

</td>

<td style="text-align:right;">

0.0038339

</td>

<td style="text-align:right;">

0.0642087

</td>

<td style="text-align:right;">

\-0.0799990

</td>

<td style="text-align:right;">

\-0.0080951

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

57.6150

</td>

<td style="text-align:right;">

0.0165731

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

240

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

1

</td>

<td style="text-align:right;">

279

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

1170

</td>

<td style="text-align:right;">

39193

</td>

<td style="text-align:right;">

3012

</td>

<td style="text-align:right;">

9390

</td>

<td style="text-align:right;">

52.46

</td>

<td style="text-align:right;">

52.6379

</td>

<td style="text-align:right;">

52.83

</td>

<td style="text-align:right;">

0.06

</td>

<td style="text-align:right;">

0.0384204

</td>

<td style="text-align:right;">

0.07

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.301680

</td>

<td style="text-align:right;">

0.603359

</td>

<td style="text-align:right;">

0.476046

</td>

<td style="text-align:right;">

0.501376

</td>

<td style="text-align:right;">

0.505950

</td>

<td style="text-align:right;">

0.510524

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.994905

</td>

<td style="text-align:right;">

0.0006370

</td>

<td style="text-align:right;">

\-0.0218349

</td>

<td style="text-align:right;">

0.0066480

</td>

<td style="text-align:right;">

0.0653442

</td>

<td style="text-align:right;">

\-0.0797706

</td>

<td style="text-align:right;">

\-0.0169246

</td>

<td style="text-align:right;">

0.478157

</td>

<td style="text-align:right;">

52.6393

</td>

<td style="text-align:right;">

0.0398239

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

245

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

438

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

119

</td>

<td style="text-align:right;">

10747

</td>

<td style="text-align:right;">

2182

</td>

<td style="text-align:right;">

9653

</td>

<td style="text-align:right;">

47.48

</td>

<td style="text-align:right;">

47.6773

</td>

<td style="text-align:right;">

47.89

</td>

<td style="text-align:right;">

0.09

</td>

<td style="text-align:right;">

0.0768502

</td>

<td style="text-align:right;">

0.10

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.285231

</td>

<td style="text-align:right;">

0.570462

</td>

<td style="text-align:right;">

0.469306

</td>

<td style="text-align:right;">

0.478714

</td>

<td style="text-align:right;">

0.482291

</td>

<td style="text-align:right;">

0.485867

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.990302

</td>

<td style="text-align:right;">

0.0011422

</td>

<td style="text-align:right;">

\-0.0344797

</td>

<td style="text-align:right;">

0.0139756

</td>

<td style="text-align:right;">

0.0663346

</td>

<td style="text-align:right;">

\-0.0794014

</td>

<td style="text-align:right;">

\-0.0294951

</td>

<td style="text-align:right;">

0.475861

</td>

<td style="text-align:right;">

47.6865

</td>

<td style="text-align:right;">

0.0860111

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

247

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

5

</td>

<td style="text-align:right;">

1290

</td>

<td style="text-align:right;">

5483

</td>

<td style="text-align:right;">

1760

</td>

<td style="text-align:right;">

45.50

</td>

<td style="text-align:right;">

45.6971

</td>

<td style="text-align:right;">

45.89

</td>

<td style="text-align:right;">

0.10

</td>

<td style="text-align:right;">

0.0962040

</td>

<td style="text-align:right;">

0.11

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.273995

</td>

<td style="text-align:right;">

0.547991

</td>

<td style="text-align:right;">

0.463901

</td>

<td style="text-align:right;">

0.466226

</td>

<td style="text-align:right;">

0.469087

</td>

<td style="text-align:right;">

0.471948

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.987998

</td>

<td style="text-align:right;">

0.0013914

</td>

<td style="text-align:right;">

\-0.0401191

</td>

<td style="text-align:right;">

0.0140348

</td>

<td style="text-align:right;">

0.0666926

</td>

<td style="text-align:right;">

\-0.0792168

</td>

<td style="text-align:right;">

\-0.0351075

</td>

<td style="text-align:right;">

0.473638

</td>

<td style="text-align:right;">

45.7138

</td>

<td style="text-align:right;">

0.1129540

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

248

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

15

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

16

</td>

<td style="text-align:right;">

2129

</td>

<td style="text-align:right;">

3666

</td>

<td style="text-align:right;">

700

</td>

<td style="text-align:right;">

44.52

</td>

<td style="text-align:right;">

44.7063

</td>

<td style="text-align:right;">

44.92

</td>

<td style="text-align:right;">

0.11

</td>

<td style="text-align:right;">

0.1052830

</td>

<td style="text-align:right;">

0.12

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.273081

</td>

<td style="text-align:right;">

0.546162

</td>

<td style="text-align:right;">

0.459372

</td>

<td style="text-align:right;">

0.462066

</td>

<td style="text-align:right;">

0.464921

</td>

<td style="text-align:right;">

0.467776

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.986881

</td>

<td style="text-align:right;">

0.0015163

</td>

<td style="text-align:right;">

\-0.0425395

</td>

<td style="text-align:right;">

0.0175110

</td>

<td style="text-align:right;">

0.0668746

</td>

<td style="text-align:right;">

\-0.0791272

</td>

<td style="text-align:right;">

\-0.0375142

</td>

<td style="text-align:right;">

0.471728

</td>

<td style="text-align:right;">

44.7280

</td>

<td style="text-align:right;">

0.1269210

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

249

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

4

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

10

</td>

<td style="text-align:right;">

13505

</td>

<td style="text-align:right;">

4517

</td>

<td style="text-align:right;">

800

</td>

<td style="text-align:right;">

43.51

</td>

<td style="text-align:right;">

43.7161

</td>

<td style="text-align:right;">

43.93

</td>

<td style="text-align:right;">

0.12

</td>

<td style="text-align:right;">

0.1148780

</td>

<td style="text-align:right;">

0.13

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.269065

</td>

<td style="text-align:right;">

0.538129

</td>

<td style="text-align:right;">

0.455004

</td>

<td style="text-align:right;">

0.457923

</td>

<td style="text-align:right;">

0.460772

</td>

<td style="text-align:right;">

0.463621

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.985691

</td>

<td style="text-align:right;">

0.0016497

</td>

<td style="text-align:right;">

\-0.0450826

</td>

<td style="text-align:right;">

0.0175471

</td>

<td style="text-align:right;">

0.0670505

</td>

<td style="text-align:right;">

\-0.0790318

</td>

<td style="text-align:right;">

\-0.0400441

</td>

<td style="text-align:right;">

0.469783

</td>

<td style="text-align:right;">

43.7437

</td>

<td style="text-align:right;">

0.1424310

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

250

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

2

</td>

<td style="text-align:right;">

426

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

1799

</td>

<td style="text-align:right;">

26307

</td>

<td style="text-align:right;">

4278

</td>

<td style="text-align:right;">

700

</td>

<td style="text-align:right;">

42.53

</td>

<td style="text-align:right;">

42.7269

</td>

<td style="text-align:right;">

42.95

</td>

<td style="text-align:right;">

0.13

</td>

<td style="text-align:right;">

0.1254170

</td>

<td style="text-align:right;">

0.14

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.266140

</td>

<td style="text-align:right;">

0.532280

</td>

<td style="text-align:right;">

0.451191

</td>

<td style="text-align:right;">

0.453797

</td>

<td style="text-align:right;">

0.456145

</td>

<td style="text-align:right;">

0.458493

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.984389

</td>

<td style="text-align:right;">

0.0017949

</td>

<td style="text-align:right;">

\-0.0478911

</td>

<td style="text-align:right;">

0.0175834

</td>

<td style="text-align:right;">

0.0672173

</td>

<td style="text-align:right;">

\-0.0789274

</td>

<td style="text-align:right;">

\-0.0428401

</td>

<td style="text-align:right;">

0.467820

</td>

<td style="text-align:right;">

42.7618

</td>

<td style="text-align:right;">

0.1603170

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

251

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

7

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

271

</td>

<td style="text-align:right;">

15028

</td>

<td style="text-align:right;">

4687

</td>

<td style="text-align:right;">

2012

</td>

<td style="text-align:right;">

41.53

</td>

<td style="text-align:right;">

41.7392

</td>

<td style="text-align:right;">

41.92

</td>

<td style="text-align:right;">

0.14

</td>

<td style="text-align:right;">

0.1375140

</td>

<td style="text-align:right;">

0.15

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.256440

</td>

<td style="text-align:right;">

0.512881

</td>

<td style="text-align:right;">

0.447223

</td>

<td style="text-align:right;">

0.448362

</td>

<td style="text-align:right;">

0.450653

</td>

<td style="text-align:right;">

0.452945

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.982918

</td>

<td style="text-align:right;">

0.0019560

</td>

<td style="text-align:right;">

\-0.0509297

</td>

<td style="text-align:right;">

0.0218205

</td>

<td style="text-align:right;">

0.0673700

</td>

<td style="text-align:right;">

\-0.0788094

</td>

<td style="text-align:right;">

\-0.0458673

</td>

<td style="text-align:right;">

0.465778

</td>

<td style="text-align:right;">

41.7812

</td>

<td style="text-align:right;">

0.1795280

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

252

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

10

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

104

</td>

<td style="text-align:right;">

661

</td>

<td style="text-align:right;">

4033

</td>

<td style="text-align:right;">

1857

</td>

<td style="text-align:right;">

40.55

</td>

<td style="text-align:right;">

40.7531

</td>

<td style="text-align:right;">

40.93

</td>

<td style="text-align:right;">

0.15

</td>

<td style="text-align:right;">

0.1512220

</td>

<td style="text-align:right;">

0.16

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.252133

</td>

<td style="text-align:right;">

0.504267

</td>

<td style="text-align:right;">

0.443405

</td>

<td style="text-align:right;">

0.442846

</td>

<td style="text-align:right;">

0.445133

</td>

<td style="text-align:right;">

0.447419

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.981266

</td>

<td style="text-align:right;">

0.0021342

</td>

<td style="text-align:right;">

\-0.0542692

</td>

<td style="text-align:right;">

0.0218653

</td>

<td style="text-align:right;">

0.0675078

</td>

<td style="text-align:right;">

\-0.0786770

</td>

<td style="text-align:right;">

\-0.0491964

</td>

<td style="text-align:right;">

0.463703

</td>

<td style="text-align:right;">

40.8038

</td>

<td style="text-align:right;">

0.2019230

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

253

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

38

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

1312

</td>

<td style="text-align:right;">

3752

</td>

<td style="text-align:right;">

582

</td>

<td style="text-align:right;">

7464

</td>

<td style="text-align:right;">

39.57

</td>

<td style="text-align:right;">

39.7627

</td>

<td style="text-align:right;">

39.94

</td>

<td style="text-align:right;">

0.17

</td>

<td style="text-align:right;">

0.1606130

</td>

<td style="text-align:right;">

0.18

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.247748

</td>

<td style="text-align:right;">

0.495497

</td>

<td style="text-align:right;">

0.437633

</td>

<td style="text-align:right;">

0.441328

</td>

<td style="text-align:right;">

0.443198

</td>

<td style="text-align:right;">

0.445068

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.979986

</td>

<td style="text-align:right;">

0.0022891

</td>

<td style="text-align:right;">

\-0.0564869

</td>

<td style="text-align:right;">

0.0219092

</td>

<td style="text-align:right;">

0.0676766

</td>

<td style="text-align:right;">

\-0.0785743

</td>

<td style="text-align:right;">

\-0.0514013

</td>

<td style="text-align:right;">

0.460124

</td>

<td style="text-align:right;">

39.8223

</td>

<td style="text-align:right;">

0.2202700

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

<tr>

<td style="text-align:left;">

SPY

</td>

<td style="text-align:left;">

2020-05-19

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

11

</td>

<td style="text-align:right;">

254

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

22

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

50

</td>

<td style="text-align:right;">

67

</td>

<td style="text-align:right;">

2797

</td>

<td style="text-align:right;">

2927

</td>

<td style="text-align:right;">

2528

</td>

<td style="text-align:right;">

38.58

</td>

<td style="text-align:right;">

38.7807

</td>

<td style="text-align:right;">

38.96

</td>

<td style="text-align:right;">

0.18

</td>

<td style="text-align:right;">

0.1784280

</td>

<td style="text-align:right;">

0.19

</td>

<td style="text-align:right;">

0

</td>

<td style="text-align:right;">

0.244667

</td>

<td style="text-align:right;">

0.489333

</td>

<td style="text-align:right;">

0.434100

</td>

<td style="text-align:right;">

0.434687

</td>

<td style="text-align:right;">

0.436553

</td>

<td style="text-align:right;">

0.438419

</td>

<td style="text-align:right;">

\-0.0065171

</td>

<td style="text-align:right;">

0.977890

</td>

<td style="text-align:right;">

0.0025081

</td>

<td style="text-align:right;">

\-0.0605062

</td>

<td style="text-align:right;">

0.0267923

</td>

<td style="text-align:right;">

0.0677777

</td>

<td style="text-align:right;">

\-0.0784063

</td>

<td style="text-align:right;">

\-0.0554131

</td>

<td style="text-align:right;">

0.457933

</td>

<td style="text-align:right;">

38.8483

</td>

<td style="text-align:right;">

0.2459960

</td>

<td style="text-align:right;">

292.55

</td>

<td style="text-align:left;">

2020-05-19 20:02:33

</td>

</tr>

</tbody>

</table>

</div>

## Cross-over and differences between `purrr` and `dplyr`

Here are some other interesting things that we can do with the nested
lists via `purrr`, and their equivalent operation on the `strikes_df`
dataframe using `dplyr`.

The intent is to gain some intuition for `purrr` using what you already
know about `dplyr`.

### Get vector of column names

``` r
strikes %>%
  map(names) %>%
  unique() %>%
  unlist()
```

    ##  [1] "ticker"           "tradeDate"        "expirDate"        "dte"             
    ##  [5] "strike"           "stockPrice"       "callVolume"       "callOpenInterest"
    ##  [9] "callBidSize"      "callAskSize"      "putVolume"        "putOpenInterest" 
    ## [13] "putBidSize"       "putAskSize"       "callBidPrice"     "callValue"       
    ## [17] "callAskPrice"     "putBidPrice"      "putValue"         "putAskPrice"     
    ## [21] "callBidIv"        "callMidIv"        "callAskIv"        "smvVol"          
    ## [25] "putBidIv"         "putMidIv"         "putAskIv"         "residualRate"    
    ## [29] "delta"            "gamma"            "theta"            "vega"            
    ## [33] "rho"              "phi"              "driftlessTheta"   "extSmvVol"       
    ## [37] "extCallValue"     "extPutValue"      "spotPrice"        "updatedAt"

This is equivalent to the following `dplyr` operation on the
`strikes_df` dataframe:

``` r
strikes_df %>%
  names
```

    ##  [1] "ticker"           "tradeDate"        "expirDate"        "dte"             
    ##  [5] "strike"           "stockPrice"       "callVolume"       "callOpenInterest"
    ##  [9] "callBidSize"      "callAskSize"      "putVolume"        "putOpenInterest" 
    ## [13] "putBidSize"       "putAskSize"       "callBidPrice"     "callValue"       
    ## [17] "callAskPrice"     "putBidPrice"      "putValue"         "putAskPrice"     
    ## [21] "callBidIv"        "callMidIv"        "callAskIv"        "smvVol"          
    ## [25] "putBidIv"         "putMidIv"         "putAskIv"         "residualRate"    
    ## [29] "delta"            "gamma"            "theta"            "vega"            
    ## [33] "rho"              "phi"              "driftlessTheta"   "extSmvVol"       
    ## [37] "extCallValue"     "extPutValue"      "spotPrice"        "updatedAt"

You can see the connection: `map(strikes, names)` applies `names` to
each sublist in `strikes`, returning a list of names for each sublist,
which we then check for a single unique case and convert to a charcter
vector via `unlist`.

In the dataframe version, we’ve already mapped each sublist to a
dataframe row. We can get the column names of the dataframe by calling
`names` directly on this object.

### Check that all elements have the same ticker

``` r
strikes %>%
  map_chr("ticker") %>%  # this makes a character vector of list elements "ticker"
  unique()
```

    ## [1] "SPY"

Calling the `purrr::map` functions on a list with the name of a common
sub-element returns the value associated with each sub-element. `map`
returns a list; here we use `map_chr` to return a character vector.

This only works if the thing being returned from the sub-element is
indeed a character.

This is equivalent to the following `dplyr` operation on the
`strikes_df` dataframe:

``` r
strikes_df %>%
  distinct(ticker) %>%
  pull()
```

    ## [1] "SPY"

In the `dplyr` dataframe version, we’ve already mapped our tickers to
their own column. So we simply call `distinct` on that column to get the
unique values. A `pull` converts the resulting tibble to a vector.

### Get the strike prices, expiries and call and put mid-prices

In this case, the `purrr` solution is somewhat convoluted:

``` r
callBids <- strikes %>%
  map_dbl("callBidPrice")

callAsks <- strikes %>%
  map_dbl("callAskPrice")

putBids <- strikes %>%
  map_dbl("putBidPrice")

putAsks <- strikes %>%
  map_dbl("putAskPrice")

data.frame(
  strike = strikes %>% map_dbl("strike"),
  expirDate = strikes %>% map_chr("expirDate"),
  callMid = map2_dbl(.x = callBids, .y = callAsks, ~{(.x + .y)/2}),
  putMid = map2_dbl(.x = putBids, .y = putAsks, ~{(.x + .y)/2})
) %>%
  pretty_print(10)
```

<div style="border: 1px solid #ddd; padding: 0px; overflow-y: scroll; height:300px; ">

<table class="table" style="margin-left: auto; margin-right: auto;">

<thead>

<tr>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

strike

</th>

<th style="text-align:left;position: sticky; top:0; background-color: #FFFFFF;">

expirDate

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callMid

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putMid

</th>

</tr>

</thead>

<tbody>

<tr>

<td style="text-align:right;">

140

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

152.600

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

145

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

147.600

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

150

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

142.610

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

155

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

137.585

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

160

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

132.620

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

165

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

127.620

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

170

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

122.600

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

175

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

117.585

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

180

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

112.610

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

185

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

107.610

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

</tbody>

</table>

</div>

Since our mapping function requires two inputs, we need to use the
`map2` functions, and must set up the inputs as a first step.

The `dplyr` equivalent on the dataframe object is much more succinct:

``` r
strikes_df %>%
  mutate(
    callMid = (callBidPrice + callAskPrice)/2,
    putMid = (putBidPrice + putAskPrice)/2
  ) %>%
  select(strike, expirDate, callMid, putMid) %>%
  pretty_print(10)
```

<div style="border: 1px solid #ddd; padding: 0px; overflow-y: scroll; height:300px; ">

<table class="table" style="margin-left: auto; margin-right: auto;">

<thead>

<tr>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

strike

</th>

<th style="text-align:left;position: sticky; top:0; background-color: #FFFFFF;">

expirDate

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

callMid

</th>

<th style="text-align:right;position: sticky; top:0; background-color: #FFFFFF;">

putMid

</th>

</tr>

</thead>

<tbody>

<tr>

<td style="text-align:right;">

140

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

152.600

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

145

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

147.600

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

150

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

142.610

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

155

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

137.585

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

160

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

132.620

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

165

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

127.620

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

170

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

122.600

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

175

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

117.585

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

180

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

112.610

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

<tr>

<td style="text-align:right;">

185

</td>

<td style="text-align:left;">

2020-05-29

</td>

<td style="text-align:right;">

107.610

</td>

<td style="text-align:right;">

0.005

</td>

</tr>

</tbody>

</table>

</div>

### Leverage the dataframe’s structure

We can also leverage the fact that a dataframe is represented as a list
of columns to use `purrr` functions directly on dataframes. These
recipes are quite useful for quickly getting to know a dataframe.

For instance, we can get the type of each column:

``` r
strikes_df %>%
  map_chr(class)
```

    ##           ticker        tradeDate        expirDate              dte 
    ##      "character"      "character"      "character"        "integer" 
    ##           strike       stockPrice       callVolume callOpenInterest 
    ##        "numeric"        "numeric"        "integer"        "integer" 
    ##      callBidSize      callAskSize        putVolume  putOpenInterest 
    ##        "integer"        "integer"        "integer"        "integer" 
    ##       putBidSize       putAskSize     callBidPrice        callValue 
    ##        "integer"        "integer"        "numeric"        "numeric" 
    ##     callAskPrice      putBidPrice         putValue      putAskPrice 
    ##        "numeric"        "numeric"        "numeric"        "numeric" 
    ##        callBidIv        callMidIv        callAskIv           smvVol 
    ##        "numeric"        "numeric"        "numeric"        "numeric" 
    ##         putBidIv         putMidIv         putAskIv     residualRate 
    ##        "numeric"        "numeric"        "numeric"        "numeric" 
    ##            delta            gamma            theta             vega 
    ##        "numeric"        "numeric"        "numeric"        "numeric" 
    ##              rho              phi   driftlessTheta        extSmvVol 
    ##        "numeric"        "numeric"        "numeric"        "numeric" 
    ##     extCallValue      extPutValue        spotPrice        updatedAt 
    ##        "numeric"        "numeric"        "numeric"      "character"

Which is equivalent to a `dplyr::summarise_all`, except that this
returns a tibble rather than a vector:

``` r
strikes_df %>%
  summarise_all(~class(.x)) 
```

    ## # A tibble: 1 x 40
    ##   ticker tradeDate expirDate dte   strike stockPrice callVolume callOpenInterest
    ##   <chr>  <chr>     <chr>     <chr> <chr>  <chr>      <chr>      <chr>           
    ## 1 chara~ character character inte~ numer~ numeric    integer    integer         
    ## # ... with 32 more variables: callBidSize <chr>, callAskSize <chr>,
    ## #   putVolume <chr>, putOpenInterest <chr>, putBidSize <chr>, putAskSize <chr>,
    ## #   callBidPrice <chr>, callValue <chr>, callAskPrice <chr>, putBidPrice <chr>,
    ## #   putValue <chr>, putAskPrice <chr>, callBidIv <chr>, callMidIv <chr>,
    ## #   callAskIv <chr>, smvVol <chr>, putBidIv <chr>, putMidIv <chr>,
    ## #   putAskIv <chr>, residualRate <chr>, delta <chr>, gamma <chr>, theta <chr>,
    ## #   vega <chr>, rho <chr>, phi <chr>, driftlessTheta <chr>, extSmvVol <chr>,
    ## #   extCallValue <chr>, extPutValue <chr>, spotPrice <chr>, updatedAt <chr>

We can also get the number of distinct values in each column using
`purrr` functions:

``` r
strikes_df %>%
  map_dbl(n_distinct)
```

    ##           ticker        tradeDate        expirDate              dte 
    ##                1                1               31               31 
    ##           strike       stockPrice       callVolume callOpenInterest 
    ##              151                1              162             1097 
    ##      callBidSize      callAskSize        putVolume  putOpenInterest 
    ##              144              165              498             1691 
    ##       putBidSize       putAskSize     callBidPrice        callValue 
    ##             1054              926             2151             2228 
    ##     callAskPrice      putBidPrice         putValue      putAskPrice 
    ##             2199             1281             2419             1311 
    ##        callBidIv        callMidIv        callAskIv           smvVol 
    ##             1907             2423             2430             2044 
    ##         putBidIv         putMidIv         putAskIv     residualRate 
    ##             2345             2425             2428             2039 
    ##            delta            gamma            theta             vega 
    ##             2137             2238             2192             2211 
    ##              rho              phi   driftlessTheta        extSmvVol 
    ##             2188             2133             2196             2016 
    ##     extCallValue      extPutValue        spotPrice        updatedAt 
    ##             2168             2413                1                1

Again, this is equivalent to a `dplyr::summarise_all`, different return
objects aside:

``` r
strikes_df %>%
  summarise_all(~n_distinct(.x))
```

    ## # A tibble: 1 x 40
    ##   ticker tradeDate expirDate   dte strike stockPrice callVolume callOpenInterest
    ##    <int>     <int>     <int> <int>  <int>      <int>      <int>            <int>
    ## 1      1         1        31    31    151          1        162             1097
    ## # ... with 32 more variables: callBidSize <int>, callAskSize <int>,
    ## #   putVolume <int>, putOpenInterest <int>, putBidSize <int>, putAskSize <int>,
    ## #   callBidPrice <int>, callValue <int>, callAskPrice <int>, putBidPrice <int>,
    ## #   putValue <int>, putAskPrice <int>, callBidIv <int>, callMidIv <int>,
    ## #   callAskIv <int>, smvVol <int>, putBidIv <int>, putMidIv <int>,
    ## #   putAskIv <int>, residualRate <int>, delta <int>, gamma <int>, theta <int>,
    ## #   vega <int>, rho <int>, phi <int>, driftlessTheta <int>, extSmvVol <int>,
    ## #   extCallValue <int>, extPutValue <int>, spotPrice <int>, updatedAt <int>

If we wanted to put both of these things together, there’s an elegant
`purrr` solution:

``` r
strikes_df %>%
  map_df(
    ~data.frame(num_distinct = n_distinct(.x), type = class(.x)),
      .id = "variable"
  )
```

    ##            variable num_distinct      type
    ## 1            ticker            1 character
    ## 2         tradeDate            1 character
    ## 3         expirDate           31 character
    ## 4               dte           31   integer
    ## 5            strike          151   numeric
    ## 6        stockPrice            1   numeric
    ## 7        callVolume          162   integer
    ## 8  callOpenInterest         1097   integer
    ## 9       callBidSize          144   integer
    ## 10      callAskSize          165   integer
    ## 11        putVolume          498   integer
    ## 12  putOpenInterest         1691   integer
    ## 13       putBidSize         1054   integer
    ## 14       putAskSize          926   integer
    ## 15     callBidPrice         2151   numeric
    ## 16        callValue         2228   numeric
    ## 17     callAskPrice         2199   numeric
    ## 18      putBidPrice         1281   numeric
    ## 19         putValue         2419   numeric
    ## 20      putAskPrice         1311   numeric
    ## 21        callBidIv         1907   numeric
    ## 22        callMidIv         2423   numeric
    ## 23        callAskIv         2430   numeric
    ## 24           smvVol         2044   numeric
    ## 25         putBidIv         2345   numeric
    ## 26         putMidIv         2425   numeric
    ## 27         putAskIv         2428   numeric
    ## 28     residualRate         2039   numeric
    ## 29            delta         2137   numeric
    ## 30            gamma         2238   numeric
    ## 31            theta         2192   numeric
    ## 32             vega         2211   numeric
    ## 33              rho         2188   numeric
    ## 34              phi         2133   numeric
    ## 35   driftlessTheta         2196   numeric
    ## 36        extSmvVol         2016   numeric
    ## 37     extCallValue         2168   numeric
    ## 38      extPutValue         2413   numeric
    ## 39        spotPrice            1   numeric
    ## 40        updatedAt            1 character

But the best I can do with `dplyr` is somewhat less elegant:

``` r
strikes_df %>%
  summarise_all(
    list(~n_distinct(.x), ~class(.x))
  ) 
```

    ## # A tibble: 1 x 80
    ##   ticker_n_distin~ tradeDate_n_dis~ expirDate_n_dis~ dte_n_distinct
    ##              <int>            <int>            <int>          <int>
    ## 1                1                1               31             31
    ## # ... with 76 more variables: strike_n_distinct <int>,
    ## #   stockPrice_n_distinct <int>, callVolume_n_distinct <int>,
    ## #   callOpenInterest_n_distinct <int>, callBidSize_n_distinct <int>,
    ## #   callAskSize_n_distinct <int>, putVolume_n_distinct <int>,
    ## #   putOpenInterest_n_distinct <int>, putBidSize_n_distinct <int>,
    ## #   putAskSize_n_distinct <int>, callBidPrice_n_distinct <int>,
    ## #   callValue_n_distinct <int>, callAskPrice_n_distinct <int>,
    ## #   putBidPrice_n_distinct <int>, putValue_n_distinct <int>,
    ## #   putAskPrice_n_distinct <int>, callBidIv_n_distinct <int>,
    ## #   callMidIv_n_distinct <int>, callAskIv_n_distinct <int>,
    ## #   smvVol_n_distinct <int>, putBidIv_n_distinct <int>,
    ## #   putMidIv_n_distinct <int>, putAskIv_n_distinct <int>,
    ## #   residualRate_n_distinct <int>, delta_n_distinct <int>,
    ## #   gamma_n_distinct <int>, theta_n_distinct <int>, vega_n_distinct <int>,
    ## #   rho_n_distinct <int>, phi_n_distinct <int>,
    ## #   driftlessTheta_n_distinct <int>, extSmvVol_n_distinct <int>,
    ## #   extCallValue_n_distinct <int>, extPutValue_n_distinct <int>,
    ## #   spotPrice_n_distinct <int>, updatedAt_n_distinct <int>, ticker_class <chr>,
    ## #   tradeDate_class <chr>, expirDate_class <chr>, dte_class <chr>,
    ## #   strike_class <chr>, stockPrice_class <chr>, callVolume_class <chr>,
    ## #   callOpenInterest_class <chr>, callBidSize_class <chr>,
    ## #   callAskSize_class <chr>, putVolume_class <chr>,
    ## #   putOpenInterest_class <chr>, putBidSize_class <chr>,
    ## #   putAskSize_class <chr>, callBidPrice_class <chr>, callValue_class <chr>,
    ## #   callAskPrice_class <chr>, putBidPrice_class <chr>, putValue_class <chr>,
    ## #   putAskPrice_class <chr>, callBidIv_class <chr>, callMidIv_class <chr>,
    ## #   callAskIv_class <chr>, smvVol_class <chr>, putBidIv_class <chr>,
    ## #   putMidIv_class <chr>, putAskIv_class <chr>, residualRate_class <chr>,
    ## #   delta_class <chr>, gamma_class <chr>, theta_class <chr>, vega_class <chr>,
    ## #   rho_class <chr>, phi_class <chr>, driftlessTheta_class <chr>,
    ## #   extSmvVol_class <chr>, extCallValue_class <chr>, extPutValue_class <chr>,
    ## #   spotPrice_class <chr>, updatedAt_class <chr>

Intuitively, you’d reach for something like this:

``` r
try(
  strikes_df %>%
    summarise_all(
      ~data.frame(num_distinct = n_distinct(.x), type = class(.x))
    )
)
```

    ## Error : Column `ticker` must be length 1 (a summary value), not 2

But we get an error related to the fact `summarise` wants to return a
single value for each variable being summarised, that is, a dataframe
with a single row.

There are probably better `dplyr` solutions out there, but this
illustrates an important point: the `purrr::map` functions are highly
customisable, able to apply a function to individual elements in a
collection, returning a data object of your choosing. `dplyr::summarise`
really shines when you need to aggregate or reduce variables to a single
value.

## Conclusion

In this post we explored the `purrr::map` functions for wrangling a data
set consisting of nested lists, as you might have if you were reading in
JSON data to R.

We also explored the cross-over and differences in use-cases for `purrr`
and `dplyr` functions.
