"""download to csv file ETF holdings"""

import pandas as pd


def get_holdings(spdr_ticker):
    
    url = f'http://www.sectorspdr.com/sectorspdr/IDCO.Client.Spdrs.Holdings/Export/ExportCsv?symbol={spdr_ticker}'
    df = pd.read_csv(url, skiprows=1).to_csv(f'{spdr_ticker}_holdings.csv', index=False)
        
    return df
    
    
if __name__ == "__main__":
    
    tickers = ['XLB', 'XLE', 'XLF', 'XLI', 'XLK', 'XLP', 'XLU', 'XLV', 'XLY']
    
    for t in tickers:
        get_holdings(t)