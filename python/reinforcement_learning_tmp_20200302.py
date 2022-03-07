#!/usr/bin/env python
# coding: utf-8

# In[1]:


import math
import time
import wandb
import spam
import pprint


# In[ ]:


import gym
env = gym.make('CartPole-v0')

for i_episode in range(1):
    observation = env.reset()
    reward = 0
    done = False
    
    for t in range(100):
        env.render()
        
        action = spam.cartpole_v0(list(observation), reward, done)

        observation, reward, done, info = env.step(action)

        
        if done:
            print("Episode finished after {} timesteps".format(t+1))
            break
env.close()


# In[ ]:




