#!/usr/bin/env python
# coding: utf-8

# In[1]:


import math
import time
import wandb
import spam
import pprint


# In[2]:


import gym
env = gym.make('CartPole-v0')

ts = []

random_policy_average_timesteps = 24.96
oppose_tilt_policy_average_timesteps = 40.82


better_than_random = 0
worse_than_random = 0
reinitialize_weights = False

while True:
    observation = env.reset()
    reward = 0
    done = False
    
    for t in range(1000):
        env.render()
        
        action = spam.cartpole_v0(observation.tolist(), reward, done, reinitialize_weights)
        reinitialize_weights = False

        observation, reward, done, info = env.step(action)

        if done:
            ts.append(t+1)
            
            if(len(ts) >= 50):
                perf = sum(ts)/len(ts)
                if(perf < random_policy_average_timesteps):
                    worse_than_random += 1
                    print("Average timesteps: \033[91m{}\033[0m".format(perf))
                else:
                    better_than_random += 1
                    print("Average timesteps: \033[92m{}\033[0m".format(perf))
                ts = []
                
                if(better_than_random + worse_than_random > 10000):
                    if(worse_than_random > better_than_random):
                        worse_than_random = 0
                        better_than_random = 0
                        reinitialize_weights = True
                        print("\033[94m\033[1mReinitializing weights\033[0m")
                        
            break
    
env.close()


# In[ ]:




