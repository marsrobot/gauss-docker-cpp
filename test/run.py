import requests
import json

url = 'http://localhost:80/sum'

my_sum = 0
for i in range(1, 1001):
    myobj = {'x': my_sum, 'y': i}
    res = requests.post(url, json = myobj)
    js = json.loads(res.text)
    my_sum = js['sum']

print('sum: {}'.format(my_sum))
