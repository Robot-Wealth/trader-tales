Use Python from R Studio

- Example: I had a Python script for scraping ETF constituents
- Do it in the same notebook as my R analysis

install.packages("reticulate")
reticulate::use_condaenv("py37")

```python
import pandas as pd

pd.DataFrame(...)
```
