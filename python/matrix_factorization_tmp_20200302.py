#!/usr/bin/env python
# coding: utf-8

# In[1]:


import math
import time
import wandb
import spam
import pprint

squared_error = spam.matrix_factorization(features=25,
	epochs=50000,
	non_negative=1,
	learning_rate=0.001,
	weight_decay=0,
	dropout=0,
	momentum_coefficient=0,
	fp_precision=24,
	fp_emax=127,
	fp_subnormal='CPFLOAT_SUBN_USE',
	fp_round='CPFLOAT_RND_SE',
	fp_flip='CPFLOAT_NO_SOFTERR',
	fp_p=0,
	fp_explim='CPFLOAT_EXPRANGE_TARG'
)
