# https://docs.google.com/spreadsheets/d/1g0jW7k-GJiHueQ0ZVYe4WilupnUkBYLVlbB9GEdqQ98/

if __name__ != '__main__': quit()
import csv
import json
import os
import re
import requests

files = os.listdir('sheets')
sheet_re = re.compile(r'Ranked AMQ Data Links - [0-9]{4} S[0-9]{1,2}.csv')
link_re = re.compile(r'https{0,1}://pastebin.com/[\w]+')

def get_year_and_season(sheet):
    return (int(sheet[24:28]),int(sheet[30:sheet.find('.')]))

def is_url(link):
    return link_re.match(link) is not None

unmatched_urls = set()

for sheet in files:
    result = sheet_re.match(sheet)
    assert result and result.string == sheet, sheet
    year,season = get_year_and_season(sheet)
    season_text = '%04d_S%02d'%(year,season)
    if not os.path.exists(season_text):
        os.mkdir(season_text)
    print('===',year,'S'+str(season),'===')
    lines = list(csv.reader(open('sheets/'+sheet,'r')))
    assert lines[0][2:5:2] == ['Central','West']
    assert lines[1] == ['Day','Date']+['Songlist','Recording']*2
    for line in lines[2:]:
        number,date,link_central,_,link_west,_ = line
        number = 'champion' if number == 'Championship' else '%02d'%int(number)
        date = [int(x) for x in date.split('/')]
        date = '%04d-%02d-%02d'%(date[2],date[0],date[1])
        # name format: amq_ranked_<region>_<year>S<season>_<number>_<date>.json
        name_end = '_%s_%s_%s.json'%(season_text,number,date)
        link_central = link_central.strip()
        link_west = link_west.strip()
        if is_url(link_central):
            i = len(link_central)-1
            while link_central[i] != '/': i -= 1
            link_central = link_central[:i] + '/raw' + link_central[i:]
            file_name = 'amq_ranked_central' + name_end
            if os.path.exists(season_text+'/'+file_name):
                print('already downloaded:',file_name)
            else:
                request = requests.get(link_central)
                if request.ok:
                    outfile = open(season_text+'/'+file_name,'w')
                    outfile.write(request.text)
                    outfile.close()
                    print('success:',file_name)
                else:
                    print('FAILED:',file_name)
                    print('url =',link_central)
        else: unmatched_urls.add(link_central)
        if is_url(link_west):
            i = len(link_west)-1
            while link_west[i] != '/': i -= 1
            link_west = link_west[:i] + '/raw' + link_west[i:]
            file_name = 'amq_ranked_west' + name_end
            if os.path.exists(season_text+'/'+file_name):
                print('already downloaded:',file_name)
            else:
                request = requests.get(link_west)
                if request.ok:
                    outfile = open(season_text+'/'+file_name,'w')
                    outfile.write(request.text)
                    outfile.close()
                    print('success:',file_name)
                else:
                    print('FAILED:',file_name)
                    print('url =',link_west)
        else: unmatched_urls.add(link_west)

print('non urls:',unmatched_urls)
