# load raw json
strikes <- readRDS("C:/Users/Kris/Downloads/SPYstrikes.RDS")

# look inside
str(strikes, max.level = 1)  # tells us we have a component named "data"

str(strikes$data, max.level = 1, list.len = 10)  # suggests we have homogenous lists of 40 elements each

str(strikes$data[[1]])  # sweet - all the elements look like they can be easily handled by R

# pull out the interesting bit
strikes <- strikes[["data"]]

library(tidyverse)

# how many observations?
length(strikes)

# are all strike sublists identically named?
strikes %>%
  map(names) %>%
  unique() %>%
  length() == 1

# make a dataframe
strikes_df <- strikes %>%
  map_df(flatten_df)  # flatten removes one level of hierarchy from a list...like unlist...but here is applied to each sub-list. flatten_df returns each sublist as a dataframe, which makes it applicable to use in map_df (which maps the function over all the elements of strikes, but requires a function that returns a dataframe) 

### Some other interesting things....

# get vector of column names
strikes_list_names <- strikes %>%
  map(names) %>%
  unique() %>%
  unlist()

# check that all elements have the same ticker
strikes %>%
  map_chr("ticker") %>%  # this makes a character vector of list elements "ticker"
  unique()