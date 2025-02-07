An assumption we often make in trading research is that the future will be at least a little like the past. 

I see a lot of beginners making this assumption implicitly without recognising that they're making it or thinking about whether it's reasonable to do so. 

That's a mistake. 

There are a couple of ways we can explore whether it's a good assumption:
- Reason and eyeball
- Data analysis

Let's look into these. 

## Reason and eyeball

The first approach is simple - eyeballing charts and applying reason.

A simple example is the equity risk premium. 

We observe total equity returns in the past being very positive. 

![equity returns chart]()

And we think we understand why - because equities are risky, they tend to trade lower than the discounted sum of their expected cash flows. Read more about the equity risk premium [here](https://robotwealth.com/three-types-of-systematic-strategy-that-work/).

So we make the assumption that they'll continue to go up, at least over the long term, in the future. 

Is that reasonable?

We can't really know for sure, but some things that can help us weigh the evidence include:
- Does it make economic sense? *Yes, human risk preferences dictate that a risky investment trade lower than an equivalent risk-free investent.*
- Do we see evidence of it in the data? *Yes, we see stocks going up in the long term.*
- Is the effect consistent over time? *Yes, but of course there are periods where stock returns were negative.*
- Is the effect consistent elsewhere (other markets)? *Yes, it shows up in nearly every developed market for which we have data.*

![chart of stock returns]()
![chart of international stock returns]()

These questions can never provide us with absolute certainty, but if they're all pointing in the right direction, then we can at least have some confidence that our assumption of persistence is reasonable. 

In the case of the equity risk premium, I think we can have confidence in our assumption of persistence simply by applying some reason and eyeballing some data. 

But that won't always be the case. What if we need to dig into the data in more depth? How do we do that?

## Using data analysis to explore the assumption of persistence

Next I want to show you some simple data analysis techniques you can use to explore the assumption of persistence. 

We'll do this in the context of portfolio construction and risk management. 

When we create portfolios, we size positions based on trying to maximise some objective - usually risk-adjusted returns. 

![portfolio construction]()

We tend to forecast our risk metrics (variance, covariance) by estimating them and assuming they tend to stay roughly the same. Is that reasonable?

Let's start with volatility persistence. 

Would it make economic sense for volatility to be persistent? Yes, I think so, because a given asset has similar risk exposures over time. 

Do we see evidence of it in the data?

To answer this using data analysis, we can look very directly by splitting our data up into periods that don't overlap, and seeing if one period's volatility is correlated with the next:

![two-period example]()

The example in the image above is easy to understand, but it might not be obvious how to do do it with a continuous time-series. This is the process in a nutshell:

- Start with daily OHLC observations because they're easy to work with 
- Estimate volatility as the range in per cent between the high and the low
- Copy the volatility time series and shift it by 1 day
- Plot a scatter of the volatility estimate against the shifted volatility estimate and see if there's some relationship

![the simplest thing slide]()

Here's how you would do this in R:

An important consideration is overlapping data. 

In our example, we had no overlapping data issues because each daily price bar gave us a single estimate of volatility for that day. 

But say we estimated volatility using the *average* range over 30 days instead. Now we have a window of 30 days' worth of price bars going into our estimate of volatility today. 

You can see how this creates relationships between subsequent daily estimates of volatility: tomorrow's estimate differs from today's estimate by only a single data point out of 30. 

We'll come back to this issue towards the end of the article. 





