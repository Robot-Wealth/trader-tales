# Martingale Account Survival

library(tidyverse)

blow_up_loss_streak <- function(initial_size = 0.01) {

    # loss streak required to lose starting capital
  
  loss_pct <- cumsum(initial_size*2^(c(0:50)))
  
  return(which(loss_pct > 1)[1])
} 

n_50 <- function(win_rate, initial_size) {
  
  # number of trades to have 50% probability of blowing up
  
  # loss streak to blow up
  streak <- blow_up_loss_streak(initial_size)
  
  # probability of streak of losses given win rate
  P_blow_up <- (1 - win_rate)^streak
  
  # number of trades for whcih probability of blowing up exceeds 50%
  n <- log(0.5)/log(1 - P_blow_up)   
  
  return(n)
} 

# plot the relationship between win rate, initial stake and n_50

win_rates <- seq(0.5, 0.65, by = 0.02)
initial_sizes <- c(0.0025, 0.005, 0.01)

df <- data.frame(
  win_rate = rep(win_rates, length(initial_sizes)), 
  initial_size = (
    map(initial_sizes, ~rep(., length(win_rates))) %>% 
    unlist()
  )
)

df <- df%>%
  mutate(n_50 = map2_dbl(df$win_rate, df$initial_size, .f = n_50)) 

df %>%
  ggplot(aes(x = win_rate, y = n_50, colour = as.factor(initial_size))) +
    geom_line() +
    labs(
      x = "Win Rate",
      y = "n 50",
      title = "Martingale Position Sizing",
      subtitle = "Number of trades for 50% probability of blowing up",
      colour = "Initial size"
    ) +
  theme_bw()


