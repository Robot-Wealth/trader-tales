# make binance perp universe
library(rwRtools)
library(tidyverse)
library(roll)
rwlab_data_auth()

futures <- rwRtools::crypto_get_binance_perps_1h()

futures %>% 
  summarise(
    min_date = min(Datetime), 
    max_date = max(Datetime)
  )

# snippet: rw crypto load ftx perpetual funding rates v0.1
perp_rate <- rwRtools::crypto_get_binance_perps_funding()

perp_rate %>%
  summarise(
    min_date = min(funding_time), 
    max_date = max(funding_time)
  )

# funding_time does not always fall exactly on the hour. So you'll want to truncate it if you need to join it on a datetime field.
perp_rate <- perp_rate %>%
  mutate(Datetime = lubridate::floor_date(lubridate::as_datetime(funding_time), 'hour')) 

# make a daily series of perp prices and funding rates - requires some data wrangling
daily <- futures %>%
  # generally will want to left join funding onto perps, or do a full join, since funding may only occur on the 8-hours, not the hour.
  left_join(
  perp_rate %>%
    mutate(Datetime = lubridate::floor_date(lubridate::as_datetime(funding_time), 'hour')) %>%
    select(-funding_time),
  by = c("Ticker", "Datetime")
  ) %>%
  mutate(date = lubridate::as_date(Datetime)) %>%
  # need to lead the funding rate to align it properly with prices - essentially this aligns funding with the closing price given by a timestamp.
  # also change the sign of the funding rate so that we have funding to long positions - easier to work with
  group_by(Ticker) %>%
  mutate(funding_rate = lead(-funding_rate)) %>%
  group_by(Ticker, date) %>%
  arrange(date) %>%
  summarise(
    open = first(Open),
    high = max(High),
    low = min(Low),
    close = last(Close),
    dollar_volume = sum(`Quote asset volume`),
    num_trades = sum(`Number of trades`),
    taker_buy_volume = sum(`Taker buy base asset volume`),
    taker_buy_quote_volumne = sum(`Taker buy quote asset volume`),
    # total funding for the day is the sum of funding accrued
    # coalesce replaces NA with 0
    funding_rate = sum(coalesce(funding_rate, 0))
  ) %>%
  ungroup() %>%
  arrange(date, Ticker)

daily %>% head

# filter on first perp date
daily <- daily %>%
  filter(date > "2019-09-10") %>%
  filter(date <= "2024-02-13") %>% 
  rename(ticker = "Ticker")

# remove LUNAUSDT from Friday 13 May 2022. Even though it continued to exist after this, there's no way you'd include it in your universe of tradable assets.
daily <- daily %>%
  filter(! (ticker == "LUNAUSDT" & date >= "2022-05-13"))


head(daily)

daily %>% 
  # select(-m2m_returns_log, -m2m_returns_simple) %>% 
  write_csv("quantifying-combining-alphas/binance_perp_daily.csv")

