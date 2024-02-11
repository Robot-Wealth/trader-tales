# make binance perp universe
# exclude anything with price < 0.01 (arbitrary)
# exclude the 20% remaining with lowest trailing monthly volume.

library(rwRtools)
library(tidyverse)
library(roll)
rwlab_data_auth()

futures <- rwRtools::crypto_get_binance_spot_1h()

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

universe <- futures %>% 
  left_join(perp_rate, by = c('Ticker','Datetime'='funding_time')) %>%
  replace_na(list(funding_rate = 0))

universe %>% head()

universe <- universe %>%
  group_by(Ticker) %>%
  arrange(Datetime) %>%
  mutate(
    m2m_returns_simple = Close/lag(Close, 1) - 1,
    m2m_returns_log = log(m2m_returns_simple + 1),
    funding_returns_simple = lead(-funding_rate),
    funding_returns_log = log(funding_returns_simple + 1),
    total_returns_simple = m2m_returns_simple + funding_returns_simple,
    total_returns_log = log(total_returns_simple + 1),
    dollar_volume = Volume*Close
  )

universe %>% head()

universe <- universe %>% 
  mutate(
    is_universe_price = min(Open, High, Low, Close) >= 0.01,
    trail_volume = roll_mean(dollar_volume, 30 * 24)
  ) %>%
  na.omit() %>%
  group_by(Datetime) %>%
  mutate(
    volume_decile = ntile(trail_volume, 10),
    is_universe_volume = volume_decile >= 3,
    is_universe = is_universe_price & is_universe_volume
  )

daily <- universe %>% 
  mutate(
    date = as_date(Datetime)
  ) %>%
  group_by(date, Ticker) %>%
  arrange(Datetime) %>% 
  summarize(
    open = first(Open),
    high = max(High),
    low = min(Low),
    close = last(Close),
    dollar_volume = sum(dollar_volume),
    m2m_returns_log = sum(m2m_returns_log),
    m2m_returns_simple = exp(m2m_returns_log) - 1,
    funding_returns_log = sum(funding_returns_log),
    funding_returns_simple = exp(funding_returns_log) - 1,
    total_returns_simple = m2m_returns_simple + funding_returns_simple,
    total_returns_log = log(total_returns_simple + 1),
    is_universe = sum(is_universe) > 0
  )

daily %>% head()

# filter on first perp date
daily <- daily %>%
  filter(date > "2019-09-10") %>%
  rename(ticker = "Ticker")

# remove LUNAUSDT from Friday 13 May 2022. Even though it continued to exist after this, there's no way you'd include it in your universe of tradable assets.
daily <- daily %>%
  filter(! (Ticker == "LUNAUSDT" & date >= "2022-05-13"))


head(daily)

daily %>% 
  select(-m2m_returns_log, -m2m_returns_simple) %>% 
  write_csv("quantifying-combining-alphas/binance_perp_daily.csv")

