import spam
spam.system('echo %TIME%')

#spam.create_forward_net((784, 30, 10))
spam.mlp_classifier((784, 30, 10), epochs=1)
