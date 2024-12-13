# SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

import pandas as pd
import sys
pd.set_option('display.max_rows', 500)
pd.set_option('display.max_columns', 500)
pd.set_option('display.width', 1000)

print(len(sys.argv),sys.argv)

if len(sys.argv) != 4:
  print("Usage: diff.py <path/to/result-bench-1.csv> <path/to/result-bench-2.csv> <path/to/result-diff.csv>")
  exit()

f_old = sys.argv[1]
f_new = sys.argv[2]
f_diff = sys.argv[3]

df_new = pd.read_csv(f_new, sep=';', index_col=False, comment='#')  
df_old = pd.read_csv(f_old, sep=';', index_col=False, comment='#')  

df_new_m = df_new.iloc[:, 6::2]
df_old_m = df_old.iloc[:, 6::2]
df_avg = (df_new_m + df_old_m)  / 2.0
df_diff_percent = (df_new_m - df_old_m)  / df_avg * 100.0

df_new_err_m = df_new.iloc[:, 7::2]
df_old_err_m = df_old.iloc[:, 7::2]
df_diff_err_percent = (df_new_err_m + df_old_err_m) / df_avg.values * 100.0

df_mean = df_diff_percent.mean(axis=0)

df_mean_w_err = [] 
for v,err in zip(df_mean.values, df_diff_err_percent.values[0]):
  df_mean_w_err.append(v)
  df_mean_w_err.append(err)

with open(f_diff, 'a') as f:
    f.write('# Average change between versions '  + f_old + ' and ' + f_new + '\n')

df_res = pd.DataFrame([df_mean_w_err], columns=['runtime change(%)', 'err(%)', 'throughput change(%)', 'err(%)', 'speedup change(%)', 'err(%)', 'messageRate change(%)', 'err(%)'])
print(df_res)
df_res.to_csv(path_or_buf=f_diff, sep=';', index=False)

