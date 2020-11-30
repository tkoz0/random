# read all json files and load them
# import this file and call get_data()

import json
import os
import re

dir_re = re.compile(r'[0-9]{4}_S[0-9]')

# amq data as a list of items formatted like this:
# {
#   "region": "western"
#   "year": 2020
#   "season": 11
#   "number": 20 (-1 for championship)
#   "date": "2020-11-21"
#   "data": (object from the file on disk)
# }
def get_data(output=False):
    data = []
    dirs = list(filter(lambda x : dir_re.match(x), os.listdir('.')))
    for dir in dirs:
        files = os.listdir(dir)
        for file in files:
            if output: print('loading:',file)
            # get year,season,num,date
            if file.startswith('amq_ranked_central_'):
                offset = 19
                central = True
            elif file.startswith('amq_ranked_west_'):
                offset = 16
                central = False
            else: assert 0, 'invalid file name format'
            year = int(file[offset:offset+4])
            season = int(file[offset+6:offset+8])
            try:
                num = int(file[offset+9:offset+11])
                date = file[offset+12:offset+22]
            except: # championship
                assert file[offset+9:].startswith('champion')
                num = -1
                date = file[offset+18:offset+28]
            obj = dict()
            obj['region'] = 'central' if central else 'western'
            obj['year'] = year
            obj['season'] = season
            obj['number'] = num
            obj['date'] = date
            obj['data'] = json.loads(open(dir+'/'+file).read())
            data.append(obj)
    return data

if __name__ == '__main__':
    data = get_data(True)
