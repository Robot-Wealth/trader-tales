# helper functions for ew_covariance_erc_optimisation.ipynb

library(Matrix)
library(patchwork)

# functions for getting prices from Yahoo Finance
source("../data_tools/yahoo_prices.R")

# EWMA covariance estimate
# note definition of lambda in line with Risk Metrics
# ie higher values of lambda put less weight on the most recent returns and more weight on historical returns.
ewma_cov <- function(x, y, lambda, initialisation_wdw = 100) {
  # TODO: 
    # check that x and y are the same length and greater than initialisation_wdw
  
  # create initialisation window and estimation window
  init_x = x[1:initialisation_wdw]
  init_y = y[1:initialisation_wdw]

  num_obs <- length(x)
  
  # initial covariance and mean return estimates
  old_cov <- cov(init_x, init_y)
  old_x <- mean(init_x)
  old_y <- mean(init_y)
  
  # preallocate output vector
  ewma_cov <- vector(mode = "numeric", length = num_obs)

  # pad with NA for initialisation window
  ewma_cov[1:initialisation_wdw] <- NA
  
  # covariance estimate
  for(i in c((initialisation_wdw+1):num_obs)) { 
    ewma_cov[i] <- lambda*old_cov + (1 - lambda)*(old_x * old_y)
    old_cov <- ewma_cov[i]
    old_x <- x[i]
    old_y <- y[i]
  }
  ewma_cov
}

# function for recovering daily covmat from long covariance dataframe
#' @param long_covs: long dataframe with column `cov` for covariance
recover_covmat <- function(long_covs, tickers, num_assets) {
  cov_mat <- matrix(rep(0, num_assets*num_assets), num_assets)
  dimnames(cov_mat) <- list(tickers, tickers)

  # recover lower triangle and diagonal
  cov_mat[lower.tri(cov_mat, diag = TRUE)] <- long_covs$cov

  # recover upper triangle as upper triangle of transpose of half-formed matrix
  cov_mat[upper.tri(cov_mat)] <- t(cov_mat)[upper.tri(cov_mat)]

  cov_mat
}

# wrangle erc weights matrix into long dataframe with dates and returns
wrangle_weights <- function(erc_wgts, dates, tickers) {
  erc_wgts <- as.data.frame(do.call(rbind, erc_wgts))
  names(erc_wgts) <- tickers

  # add Date column and pivot long
  erc_wgts %>%
    bind_cols(dates) %>%
    pivot_longer(-Date, names_to = "Ticker", values_to = "weight") %>%
    # join returns
    left_join(
      returns %>% select(Ticker, Date, simple_return, fwd_simple_return),
      by = c("Ticker", "Date")
    )
}

#' @param weights: long dataframe of Date, Ticker, weight, fwd_simple_return
show_performance <- function(weights, title) {
  port_returns <- weights %>%
    na.omit() %>%
    group_by(Date) %>%
    summarise(
      port_return = sum(fwd_simple_return*weight/100.)
    ) 

  portfolio_returns_plot <- port_returns %>%
    mutate(port_cum_return = cumprod(1+port_return)) %>%
    ggplot(aes(x = Date, y = port_cum_return)) +
      geom_line() +
      labs(
        x = "Date",
        y = "Portfolio Return"
      ) 

  weights_plot <- weights %>%
    ggplot(aes(x = Date, y = weight, fill = Ticker)) +
      geom_area() +
      labs(
        x = "Date",
        y = "Weight",
        fill = ""
      ) +
      theme(legend.position = "bottom")

  plt <- portfolio_returns_plot / weights_plot +
    plot_annotation(title = title) +
    plot_layout(heights = c(2,1))
  print(plt)

  port_returns %>%
    summarise(
      Ann.Return = 252*mean(port_return),
      Ann.Vol = sqrt(252)*sd(port_return),
      Ann.Sharpe = Ann.Return/Ann.Vol
    )
}

#' @param weights: long dataframe of Date, Ticker, erc_wgt, fwd_simple_return
calc_port_vol <- function(weights) {
  weights %>%
    na.omit() %>%
    group_by(Date) %>%
    summarise(
      port_return = sum(fwd_simple_return*weight/100.)
    ) %>%
    ungroup() %>%
    summarise(Ann.Vol = sqrt(252)*sd(port_return)) %>%
    pull(Ann.Vol)
}

# Function for making a PSD matrix. This is a bit hacky and we'll only use it when Matrix::nearPD fails to converge.
get_near_psd <- function(mat) {
  # make symmetric
  mat = (mat + t(mat))/2

  # for negative eigenvalues to zero
  eigs <- eigen(mat)
  eigs$values[eigs$values < 0] <- 0

  # reconstruct PSD matrix from eigenvalues and eigenvectors
  near_psd <- eigs$vectors %*% diag(eigs$values) %*% t(eigs$vectors)
  if(!isSymmetric(near_psd)) {
    near_psd = (near_psd + t(near_psd))/2
  }
  near_psd
}