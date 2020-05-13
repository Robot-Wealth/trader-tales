void plotHistogram(string Name, var Val, var Step, var Weight, int Color)
{
	var Bucket = floor(Val/Step);
	plotBar(Name, Bucket, Step*Bucket, Weight, SUM+BARS+LBL2, Color);	
}

function run()
{
	setf(PlotMode, PL_FINE);
	StartDate = 2010;
	Capital = 10000;
	BarPeriod = 1440;
	NumTotalCycles = 1000;
	
	if(Equity - MarginVal < Capital) 		
		Lots = 0;
	
	// random trading strategy 
	Stop = TakeProfit = ATR(100);
	Lots = pow(2, LossStreakTotal);  // Martingale position sizing
	
	if(NumOpenTotal == 0) {
		if(random() < 0)
			enterShort();
		else 
			enterLong();
	}
	set(PLOTNOW);
	ColorDD = 0;
	if(is(EXITRUN)) 
	{
		var finalValue = max(0, Equity);
		plotHistogram("Account Equity", finalValue, 5000, 1, RED);   
	}
}