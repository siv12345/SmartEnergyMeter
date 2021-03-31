#!/usr/bin/env python
# coding: utf-8

# In[1]:


import numpy as np # linear algebra
import matplotlib.pyplot as plt
import statsmodels.api as sm
import seaborn as sns


# In[2]:


df = pd.read_csv('30days.csv', skiprows=3)


# In[3]:


df.head()


# In[4]:


df.describe()


# In[5]:


df.columns


# In[6]:


df.isnull().sum()


# In[7]:


df.isna().sum()


# In[8]:


df=df.drop(['result'], axis=1)
df=df.drop(['Unnamed: 0'], axis=1)
df=df.drop(['device'], axis=1)


# In[9]:


df.head()


# In[10]:


df_0 = df.loc[df['table'] == 0]
df_1 = df.loc[df['table'] == 1]
df_2 = df.loc[df['table'] == 2]
df_3 = df.loc[df['table'] == 3]
df_4 = df.loc[df['table'] == 4]
df_5 = df.loc[df['table'] == 5]
df_6 = df.loc[df['table'] == 6]
df_7 = df.loc[df['table'] == 7]
df_8 = df.loc[df['table'] == 8]
df_9 = df.loc[df['table'] == 9]
df_10 = df.loc[df['table'] == 10]
df_11 = df.loc[df['table'] == 11]
df_12 = df.loc[df['table'] == 12]
df_13 = df.loc[df['table'] == 13]
df_14 = df.loc[df['table'] == 14]
df_15 = df.loc[df['table'] == 15]
df_16 = df.loc[df['table'] == 16]
df_17 = df.loc[df['table'] == 17]


# In[11]:


df_10.head()


# In[12]:


df['table'].unique() #table 2 and 3 both for frequency, table 7 power factor


# In[13]:


df['_field'].unique()


# In[14]:


df_t1 = df[['table', '_field', '_measurement']]
df_t1.groupby(['table', '_field', '_measurement']).size().reset_index()


# In[15]:


#df_0.plot.line(fontsize=10, title="pf Trends",  x='_time', y='_value',figsize=(15, 7));
#df_7.plot.line(fontsize=10, title="pf Trends",  x='_time', y='_value',figsize=(15, 7));
#df_14.plot.line(fontsize=10, title="pf Trends",  x='_time', y='_value',figsize=(15, 7));


# In[16]:


plt.figure(figsize=(30, 15))
plt.title("Pf Trends", fontsize = 30)
plt.plot(df_0['_time'], df_0['_value'], label = "Phase 1")
plt.plot(df_7['_time'], df_7['_value'], label = "Phase 2")
plt.plot(df_14['_time'], df_14['_value'], label = "Phase 3")
plt.legend(fontsize=20)
plt.show()


# In[17]:


df_s = pd.read_csv('30days_solar.csv', skiprows=3)


# In[18]:


df_s.head()


# In[19]:


df_s=df_s.drop(['result'], axis=1)
df_s=df_s.drop(['Unnamed: 0'], axis=1)
df_s=df_s.drop(['Device'], axis=1)


# In[20]:


df_s.head()


# In[21]:


df_st = df_s[['table', '_field', '_measurement']]
df_st.groupby(['table', '_field', '_measurement']).size().reset_index()


# In[22]:


df_s0 = df_s.loc[df['table'] == 0]
df_s1 = df_s.loc[df['table'] == 1]
df_s2 = df_s.loc[df['table'] == 2]
df_s3 = df_s.loc[df['table'] == 3]


# In[79]:


plt.figure(figsize=(30, 15))
plt.title("Solar Trends", fontsize = 30)
#plt.plot(df_s0['_time'], df_s0['_value'], label = "Current")
#plt.plot(df_s1['_time'], df_s1['_value'], label = "Energy")
#plt.plot(df_s2['_time'], df_s2['_value'], label = "Voltage")
plt.plot(df_s3['_time'], df_s3['_value'], label = "Power")
plt.legend(fontsize=20)
plt.show()


# In[24]:


df_s0.count()


# In[25]:


df.info()


# In[26]:


df.describe()


# In[27]:


df['table'].unique()


# In[28]:


df['_value'].unique()


# In[29]:


df['_value'] = df['_value'].astype(np.int32)


# In[30]:


df['_start'] = pd.to_datetime(df['_start'], format = '%Y-%m-%dT%H:%M:%S')
df['_stop'] = pd.to_datetime(df['_stop'], format = '%Y-%m-%dT%H:%M:%S')
df['_time'] = pd.to_datetime(df['_time'], format = '%Y-%m-%dT%H:%M:%S')


# In[31]:


df['_start'].head()


# In[32]:


df['_stop'].head()


# In[33]:


df['_time'].head()


# In[34]:


df.plot.line(x = '_time', y = '_value')
plt.show()


# In[35]:


df_0 = df.loc[df['table'] == 0]
df_1 = df.loc[df['table'] == 1]
df_2 = df.loc[df['table'] == 2]
df_3 = df.loc[df['table'] == 3]
df_4 = df.loc[df['table'] == 4]
df_5 = df.loc[df['table'] == 5]
df_6 = df.loc[df['table'] == 6]
df_7 = df.loc[df['table'] == 7]
df_8 = df.loc[df['table'] == 8]
df_9 = df.loc[df['table'] == 9]
df_10 = df.loc[df['table'] == 10]
df_11 = df.loc[df['table'] == 11]
df_12 = df.loc[df['table'] == 12]
df_13 = df.loc[df['table'] == 13]
df_14 = df.loc[df['table'] == 14]
df_15 = df.loc[df['table'] == 15]
df_16 = df.loc[df['table'] == 16]
df_17 = df.loc[df['table'] == 17]


# In[36]:


df_power_ph1 = df_8
df_energy_ph1 = df_10


# In[37]:


df_power_ph1.plot.line(x = '_time', y = '_value')
plt.show()


# In[38]:


to_plot_daily_variation = df_power_ph1


# In[39]:


df['_start'].unique()


# In[40]:


df['_stop'].unique()


# In[41]:


df['_time'].unique()


# In[42]:


time_day = df_8
time_day = time_day.drop(['table', '_start', '_stop', '_value', 'SSID', '_field', '_measurement'], axis = 1)


# In[43]:


time_day.head()


# In[44]:


time = pd.to_datetime(time_day['_time'], format = '%Y-%m-%d')
time_day = time_day.drop(['_time'], axis=1)
time_day=time_day.join(time)


# In[45]:


time_day['_date'] = time_day['_time'].dt.date
time_day['_day'] = time_day['_time'].dt.day


# In[46]:


time_day


# In[47]:


time_day['_day'].unique()


# In[48]:


to_plot_daily_variation.head()


# In[49]:


to_plot_daily_variation=to_plot_daily_variation.drop(['_time'], axis = 1)
to_plot_daily_variation=to_plot_daily_variation.join(time_day['_date'])
to_plot_daily_variation=to_plot_daily_variation.join(time_day['_day'])


# In[50]:


to_plot_daily_variation.head()


# In[51]:


to_plot_daily_variation=to_plot_daily_variation.drop(['table','_start','_stop','SSID','_measurement', '_field'], axis = 1)


# In[52]:


to_plot_daily_variation


# In[53]:


to_plot_daily_variation_1 = to_plot_daily_variation.groupby(['_date'], as_index=False).sum()
to_plot_daily_variation_2 = to_plot_daily_variation.groupby(['_day'], as_index=False).sum()


# In[54]:


to_plot_daily_variation_1.head()


# In[55]:


to_plot_daily_variation_2.head()


# In[56]:


to_plot_daily_variation.columns


# In[57]:


df_power_ph1.plot.line(x = '_time', y = '_value')
plt.show()


# In[58]:


to_plot_daily_variation_2.plot.line(x = '_day', y = '_value')
plt.show()


# In[59]:


sns.barplot(x = '_day', y = '_value', data = to_plot_daily_variation_2)
#plt.figure(figsize=(60, 35))
plt.show()


# In[60]:


#sns.barplot(x = '_date', y = '_value', data = to_plot_daily_variation_1)
#plt.figure(figsize=(60, 35))
#plt.show()


# In[61]:


#rider = to_plot_daily_variation[['_value']]
rider = df_power_ph1[['_value']]


# In[62]:


rider.rolling(6).mean().plot(figsize=(20,10), linewidth=5, fontsize=20)
plt.show()


# In[63]:


rider.diff(periods=4).plot(figsize=(20,10), linewidth=5, fontsize=20)
plt.show()


# In[64]:


#pd.plotting.autocorrelation_plot(to_plot_daily_variation_2['_value'])
pd.plotting.autocorrelation_plot(df_power_ph1['_value'])

plt.show()


# In[65]:


#pd.plotting.lag_plot(to_plot_daily_variation_2['_value'])
#pd.plotting.lag_plot(df_power_ph1['_value'])
#plt.show()


# In[66]:


#to_plot_daily_variation= to_plot_daily_variation.set_index('_value')


# In[67]:


#mod = sm.tsa.SARIMAX(to_plot_daily_variation_2['_value'], trend='n', order=(0,1,0), seasonal_order=(1,1,1,28))
mod = sm.tsa.SARIMAX(df_power_ph1['_value'], trend='n', order=(0,1,1), seasonal_order=(1,1,1,28))
results = mod.fit()
print(results.summary())


# In[68]:


#to_plot_daily_variation_2['forecast'] = results.predict(start = 500, end= 522, dynamic= True)  
#to_plot_daily_variation_2[['_value', 'forecast']].plot(figsize=(12, 8))
df_power_ph1['forecast'] = results.predict(start = 150, end= 170, dynamic= True)  
df_power_ph1[['_value', 'forecast']].plot(figsize=(12, 8))

plt.show()


# In[ ]:





# In[ ]:



