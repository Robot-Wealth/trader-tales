// Click [Test] for exporting price data to a .csv file in the Data folder
// The records are stored in the format: time, open, high, low, close ...
// f.i. "31.12.12 00:00, 1.32205, 1.32341, 1.32157, 1.32278 ..."

//function for Z-score transformation
var zscore(vars data, int period) {
	return (data[0] - SMA(data, period))/StdDev(data, period);
}

//function for calculating the n-period IBS
var nperIBS(vars high, vars low, vars close, int period) {
	int i;
	var ibs;
	for (i=0; i<period; i++) {
		ibs += (close[i] - low[i])/(high[i] - low[i]);
	}

	return (ibs/period);
}

function run() {  
StartDate = 2009;  
EndDate = 2019;
LookBack = 400; 

asset("EUR/USD");
BarPeriod = 60;
FrameOffset = 9;	
AssetZone = WET; 
TimeFrame = AssetFrame; // function runs every hour, but daily bars change at FrameOffset

int period = 50; //lookback for calculating normalized/standardized values

vars Price = series(price());
vars High = series(priceHigh());
vars Low = series(priceLow());
vars Close = series(priceClose());

vars logPrice = series(log(Price[0])); 
vars logOpen = series(log(priceOpen()));
vars logHigh = series(log(High[0]));
vars logLow = series(log(Low[0]));
vars logClose = series(log(Close[0]));
vars logDeltaClose = series(logClose[0] - logClose[1]);

// ATR based on log prices
vars logATR = series(ATR(logOpen, logHigh, logLow, logClose, 7));

//Trend variable
vars trendSlow = series(LowPass(Price, 100));
vars trendFast = series(LowPass(Price,20));
vars trend = series(trendFast[0] - trendSlow[0]);
vars trendNorm = series(Normalize(trend, period));
vars trendZscore = series(zscore(trend, period));

//Linear regression of log price / ATR
vars velocity3 = series(TSF(logPrice, 3)/ATR(logOpen, logHigh, logLow, logClose, 7));
vars velocity5 = series(TSF(logPrice, 5)/ATR(logOpen, logHigh, logLow, logClose, 7));
vars velocity10 = series(TSF(logPrice, 10)/ATR(logOpen, logHigh, logLow, logClose, 7));

vars velocity3N = series(Normalize(velocity3, period));
vars velocity5N = series(Normalize(velocity5, period));
vars velocity10N = series(Normalize(velocity10, period));

//Quadratic regression of log price / ATR
vars accel3 = series(polyfit(0, logPrice, 3, 2, 1)/ATR(logOpen, logHigh, logLow, logClose, 7));
vars accel5 = series(polyfit(0, logPrice, 5, 2, 1)/ATR(logOpen, logHigh, logLow, logClose, 7));
vars accel10 = series(polyfit(0, logPrice, 10, 2, 1)/ATR(logOpen, logHigh, logLow, logClose, 7));

vars accel3N = series(Normalize(accel3, period));
vars accel5N = series(Normalize(accel5, period));
vars accel10N = series(Normalize(accel10, period));

//Cubic regression of log price / ATR
vars cubic10 = series(polyfit(0, logPrice, 10, 3, 1)/ATR(logOpen, logHigh, logLow, logClose, 7));
vars cubic25 = series(polyfit(0, logPrice, 25, 3, 1)/ATR(logOpen, logHigh, logLow, logClose, 7));
vars cubic50 = series(polyfit(0, logPrice, 50, 3, 1)/ATR(logOpen, logHigh, logLow, logClose, 7));

vars cubic10N = series(Normalize(cubic10, period));
vars cubic25N = series(Normalize(cubic25, period));
vars cubic50N = series(Normalize(cubic50, period));

//Price momentum
vars mom3 = series((logDeltaClose[0] - logDeltaClose[3])/sqrt(Moment(logDeltaClose, 100, 2)));
vars mom5 = series((logDeltaClose[0] - logDeltaClose[5])/sqrt(Moment(logDeltaClose, 100, 2)));
vars mom10 = series((logDeltaClose[0] - logDeltaClose[10])/sqrt(Moment(logDeltaClose, 100, 2)));

vars mom3N = series(Normalize(mom3, period));
vars mom5N = series(Normalize(mom5, period));
vars mom10N = series(Normalize(mom10, period));

//Close minus MA
vars closeDevMA3 = series(log(Close[0]/SMA(Close, 3))/ATR(logOpen, logHigh, logLow, logClose, 7));
vars closeDevMA5 = series(log(Close[0]/SMA(Close, 5))/ATR(logOpen, logHigh, logLow, logClose, 7));
vars closeDevMA10 = series(log(Close[0]/SMA(Close, 10))/ATR(logOpen, logHigh, logLow, logClose, 7));

vars closeDevMA3N = series(Normalize(closeDevMA3, period));
vars closeDevMA5N = series(Normalize(closeDevMA5, period));
vars closeDevMA10N = series(Normalize(closeDevMA10, period));

//Linear forecast dev
vars linDev3 = series((logPrice[0] - velocity3[0])/ATR(logOpen, logHigh, logLow, logClose, 7));
vars linDev5 = series((logPrice[0] - velocity5[0])/ATR(logOpen, logHigh, logLow, logClose, 7));
vars linDev10 = series((logPrice[0] - velocity10[0])/ATR(logOpen, logHigh, logLow, logClose, 7));

vars linDev3N = series(Normalize(linDev3, period));
vars linDev5N = series(Normalize(linDev5, period));
vars linDev10N = series(Normalize(linDev10, period));

// Quad forecast dev
vars quadDev3 = series((logPrice[0] - accel3[0])/ATR(logOpen, logHigh, logLow, logClose, 7));
vars quadDev5 = series((logPrice[0] - accel5[0])/ATR(logOpen, logHigh, logLow, logClose, 7));
vars quadDev10 = series((logPrice[0] - accel10[0])/ATR(logOpen, logHigh, logLow, logClose, 7));

vars quadDev3N = series(Normalize(quadDev3, period));
vars quadDev5N = series(Normalize(quadDev5, period));
vars quadDev10N = series(Normalize(quadDev10, period));

// cubic forecast dev
vars cubicDev3 = series((logPrice[0] - accel3[0])/ATR(logOpen, logHigh, logLow, logClose, 7));
vars cubicDev5 = series((logPrice[0] - accel5[0])/ATR(logOpen, logHigh, logLow, logClose, 7));
vars cubicDev10 = series((logPrice[0] - accel10[0])/ATR(logOpen, logHigh, logLow, logClose, 7));

vars cubicDev3N = series(Normalize(cubicDev3, period));
vars cubicDev5N = series(Normalize(cubicDev5, period));
vars cubicDev10N = series(Normalize(cubicDev10, period));

// abs price change oscillator
vars apc3 = series((Moment(logDeltaClose, 3, 1) - Moment(logDeltaClose, 100, 1))/ATR(logOpen, logHigh, logLow, logClose, 100));
vars apc5 = series((Moment(logDeltaClose, 5, 1) - Moment(logDeltaClose, 100, 1))/ATR(logOpen, logHigh, logLow, logClose, 100));
vars apc10 = series((Moment(logDeltaClose, 10, 1) - Moment(logDeltaClose, 100, 1))/ATR(logOpen, logHigh, logLow, logClose, 100));

vars apc3N = series(Normalize(apc3, period));
vars apc5N = series(Normalize(apc5, period));
vars apc10N = series(Normalize(apc10, period));

//Price variance ratio
vars priceVarRatSlow = series(Moment(logPrice, 20, 2)/Moment(logPrice, 100, 2));
vars priceVRSNorm = series(Normalize(priceVarRatSlow, period));
vars priceVRSZscore = series(zscore(priceVarRatSlow, period));

vars priceVarRatFast = series(Moment(logPrice, 10, 2)/Moment(logPrice, 20, 2));
vars priceVRFNorm = series(Normalize(priceVarRatFast, period));
vars priceVRFZscore = series(zscore(priceVarRatFast, period));

vars priceVarRat3 = series(Moment(logPrice, 3, 2)/Moment(logPrice, 100, 2));
vars priceVarRat5 = series(Moment(logPrice, 5, 2)/Moment(logPrice, 100, 2));
vars priceVarRat10 = series(Moment(logPrice, 10, 2)/Moment(logPrice, 100, 2));

vars priceVarRat3N = series(Normalize(priceVarRat3, period));
vars priceVarRat5N = series(Normalize(priceVarRat5, period));
vars priceVarRat10N = series(Normalize(priceVarRat10, period));

// delta price variance ratio
vars deltaPVR3 = series(priceVarRat3[0] - priceVarRat3[3]);
vars deltaPVR5 = series(priceVarRat5[0] - priceVarRat5[5]);
vars deltaPVR10 = series(priceVarRat10[0] - priceVarRat10[10]);

vars deltaPVR3N = series(Normalize(deltaPVR3, period));
vars deltaPVR5N = series(Normalize(deltaPVR5, period));
vars deltaPVR10N = series(Normalize(deltaPVR10, period));

//ATR ratio
vars atrRatSlow = series(ATR(20)/ATR(100));
vars atrRatSlowNorm = series(Normalize(atrRatSlow, period));
vars atrRatSlowZscore = series(zscore(atrRatSlow, period));

vars atrRatFast = series(ATR(10)/ATR(20));
vars atrRatFastNorm = series(Normalize(atrRatFast, period));
vars atrRatFastZscore = series(zscore(atrRatFast, period));

vars atrRat3 = series(ATR(3)/ATR(100));
vars atrRat5 = series(ATR(5)/ATR(100));
vars atrRat10 = series(ATR(10)/ATR(100));

vars atrRat3N = series(Normalize(atrRat3, period));
vars atrRat5N = series(Normalize(atrRat5, period));
vars atrRat10N = series(Normalize(atrRat10, period));

// delta ATR ratio
vars deltaATRrat3 = series(atrRat3[0] - atrRat3[3]);
vars deltaATRrat5 = series(atrRat5[0] - atrRat5[5]);
vars deltaATRrat10 = series(atrRat10[0] - atrRat10[10]);

vars deltaATRrat3N = series(Normalize(deltaATRrat3, period));
vars deltaATRrat5N = series(Normalize(deltaATRrat5, period));
vars deltaATRrat10N = series(Normalize(deltaATRrat10, period));

//Bollinger width
vars bWidthSlow = series(log(sqrt(Moment(Price, 100, 2))/Moment(Price, 100, 1)));
vars bWidthSlowNorm = series(Normalize(bWidthSlow, period));
vars bWidthSlowZscore = series(zscore(bWidthSlow, period));

vars bWidthFast = series(log(sqrt(Moment(Price, 20, 2))/Moment(Price, 20, 1)));
vars bWidthFastNorm = series(Normalize(bWidthFast, period));
vars bWidthFastZscore = series(zscore(bWidthFast, period));

vars bWdith3 = series(log(sqrt(Moment(Price, 3, 2))/Moment(Price, 3, 1)));
vars bWdith5 = series(log(sqrt(Moment(Price, 5, 2))/Moment(Price, 5, 1)));
vars bWdith10 = series(log(sqrt(Moment(Price, 10, 2))/Moment(Price, 10, 1)));

vars bWdith3N = series(Normalize(bWdith3, period));
vars bWdith5N = series(Normalize(bWdith5, period));
vars bWdith10N = series(Normalize(bWdith10, period));

// delta bollinger width
vars deltabWdith3 = series(bWdith3[0] - bWdith3[3]);
vars deltabWdith5 = series(bWdith5[0] - bWdith5[5]);
vars deltabWdith10 = series(bWdith10[0] - bWdith10[10]);

vars deltabWidth3N = series(Normalize(deltabWdith3, period));
vars deltabWidth5N = series(Normalize(deltabWdith5, period));
vars deltabWidth10N = series(Normalize(deltabWdith10, period));

//Close to close
vars ctoC = series(log((priceClose(0)/priceClose(1))));
vars ctoCNorm = series(Normalize(ctoC, period));
vars ctoCZscore = series(zscore(ctoC, period));

//Distance from 10-day high
vars highDist = series(HH(10,0)/priceClose(0));
vars highDistNorm = series(Normalize(highDist, period));
vars highDistZscore = series(zscore(highDist, period));

//Distance from 10-day low
vars lowDist = series(LL(10,0)/priceClose(0));
vars lowDistNorm = series(Normalize(lowDist, period));
vars lowDistZscore = series(zscore(lowDist, period));

//Close to long-term filtered price
vars trendDevSlow = series(log(priceClose(0)/LowPass(Price, 100)));
vars trendDevSlowNorm = series(Normalize(trendDevSlow, period));
vars trendDevSlowZscore = series(zscore(trendDevSlow, period));

vars trendDevFast = series(log(priceClose(0)/LowPass(Price, 20)));
vars trendDevFastNorm = series(Normalize(trendDevFast, period));
vars trendDevFastZscore = series(zscore(trendDevFast, period));


//ATR
vars ATR3 = series(ATR(3));
vars ATR5 = series(ATR(5));
vars ATR10 = series(ATR(10));
vars ATRWeek = series(ATR(7));
vars ATRFast = series(ATR(25));
vars ATRMid = series(ATR(100));
vars ATRSlow = series(ATR(250));

vars ATR3N = series(Normalize(ATR3, period));
vars ATR5N = series(Normalize(ATR5, period));
vars ATR10N = series(Normalize(ATR10, period));
vars ATRFastN = series(Normalize(ATRFast, period));

//Market Meanness Index
//vars MMISlow = series(MMI(Price, 500));
vars MMIMod = series(MMI(Price, 200)/100);
vars MMIModSmooth = series(Smooth(MMIMod, 10));
vars MMIFast = series(MMI(Price, 100)/100);
vars MMIFastSmooth = series(Smooth(MMIFast, 10));
vars MMIFaster = series(MMI(Price, 50)/100);
vars MMIFasterSmooth = series(Smooth(MMIFaster, 10));
vars MMIFastest = series(MMI(Price, 20)/100);
vars MMIFastestSmooth = series(Smooth(MMIFastest, 10));

vars MMIFasterN = series(Normalize(MMIFaster, period));
vars MMIFastestN = series(Normalize(MMIFastest, period));

//MMI deviation
vars deltaMMIFastest3 = series(MMIFastest[0] - MMIFastest[3]);
vars deltaMMIFastest5 = series(MMIFastest[0] - MMIFastest[5]);
vars deltaMMIFastest10 = series(MMIFastest[0] - MMIFastest[10]);

vars deltaMMIFastest3N = series(Normalize(deltaMMIFastest3, period));
vars deltaMMIFastest5N = series(Normalize(deltaMMIFastest5, period));
vars deltaMMIFastest10N = series(Normalize(deltaMMIFastest10, period));


//Hurst exponent
vars HurstSlow = series(Hurst(Price, 200));
vars HurstMod = series(Hurst(Price, 100));
vars HurstFast = series(Hurst(Price, 50));
vars HurstFaster = series(Hurst(Price, 20)); //20 is the minimum time period for the Hurst exponent

//Time series forecast
//vars tsfSlow = series(Normalize(series(TSF(Price, 200) - priceClose(0)), 50));
//vars tsfMod = series(Normalize(series(TSF(Price, 100) - priceClose(0)), 50));
//vars tsfFast = series(Normalize(series(TSF(Price, 50) - priceClose(0)), 50));
//vars tsfFaster = series(Normalize(series(TSF(Price, 20) - priceClose(0)), 50));
//vars tsfFastest = series(Normalize(series(TSF(Price, 10) - priceClose(0)), 50));


//n-period IBS
vars ibsOne = series(nperIBS(High, Low, Close, 1));

vars ibsOneN = series(Normalize(ibsOne, period));

//target
vars target = series(100*(priceClose(0)-priceClose(1))/ATR(100)); //price change normalized to ATR)

//  
char line[1000];  
char header[1000];
//get the values for previous day - using the values from the day the trade is entered introduces look ahead bias

sprintf(line,"%02i.%02i.%02i, %2.3f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f\n", 
day(),month(),year(), target[0], priceHigh(1),priceLow(1),priceClose(1), 
velocity3N[1], velocity5N[1], velocity10N[1],
accel3N[1], accel5N[1], accel10N[1], 
cubic10N[1], cubic25N[1], cubic50N[1], 
mom3N[1], mom5N[1], mom10N[1], 
closeDevMA3N[1], closeDevMA5N[1], closeDevMA10N[1], 
linDev3N[1], linDev5N[1], linDev10N[1], 
quadDev3N[1], quadDev5N[1], quadDev10N[1], 
cubicDev3N[1], cubicDev5N[1], cubicDev10N[1], 
apc3N[1], apc5N[1], apc10N[1], 
priceVarRat3N[1], priceVarRat5N[1], priceVarRat10N[1], 
deltaPVR3N[1], deltaPVR5N[1], deltaPVR10N[1], 
atrRat3N[1], atrRat5N[1], atrRat10N[1], 
deltaATRrat3N[1], deltaATRrat5N[1], deltaATRrat10N[1], 
bWdith3N[1], bWdith5N[1], bWdith10N[1], 
deltabWidth3N[1], deltabWidth5N[1], deltabWidth10N[1], 
MMIFasterN[1], MMIFastestN[1],
deltaMMIFastest3N[1], deltaMMIFastest5N[1], deltaMMIFastest10N[1], 
HurstMod[1], HurstFast[1], HurstFaster[1],
ibsOneN[1],
trendNorm[1], 
ctoCNorm[1], highDistNorm[1], lowDistNorm[1], trendDevSlowNorm[1], trendDevFastNorm[1], priceVRSNorm[1], priceVRFNorm[1], atrRatSlowNorm[1], atrRatFastNorm[1], bWidthSlowNorm[1], bWidthFastNorm[1], ATRWeek[1], ATRFast[1], ATRMid[1], ATRSlow[1]);

//
if(is(INITRUN)) {
    file_delete("Data\\variables.csv");  
    sprintf(header, "DDMMYYYY, target, H, L, C, velocity3N, velocity5N, velocity10N, accel3N, accel5N, accel10N, cubic10N, cubic25N, cubic50N,	mom3N, mom5N, mom10N, closeDevMA3N, closeDevMA5N, closeDevMA10N, linDev3N, linDev5N, linDev10N, quadDev3N, quadDev5N, quadDev10N, cubicDev3N, cubicDev5N, cubicDev10N, apc3N, apc5N, apc10N, priceVarRat3N, priceVarRat5N, priceVarRat10N, deltaPVR3N, deltaPVR5N, deltaPVR10N, atrRat3N, atrRat5N, atrRat10N, deltaATRrat3N, deltaATRrat5N, deltaATRrat10N, bWdith3N, bWdith5N, bWdith10N, deltabWidth3N, deltabWidth5N, deltabWidth10N, MMIFasterN, MMIFastestN, 	deltaMMIFastest3N, deltaMMIFastest5N, deltaMMIFastest10N, HurstMod, HurstFast, HurstFaster, ibsOneN, trendNorm, ctoCNorm, highDistNorm, lowDistNorm, trendDevSlowNorm, trendDevFastNorm, priceVRSNorm, priceVRFNorm, atrRatSlowNorm, atrRatFastNorm, bWidthSlowNorm, bWidthFastNorm, ATRWeek, ATRFast, ATRMod, ATRSlow\n");
    	
    file_append("Data\\variables.csv", header);
    }
else if(lhour(AssetZone) == FrameOffset) {
	file_append("Data\\variables.csv",line);
	// printf("\n%02i.%02i.%02i %02i:%02i %0.5f %.3f", day(),month(),year(), lhour(AssetZone), minute(), priceClose(), target[0]);
}

}