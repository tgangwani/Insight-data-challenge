#!/usr/bin/env python

# Author - Tanmay Gangwani
# This is a python3 script 

import orm # this module (in c++) contains all the graph functionality
import pprint
import json, sys
import calendar
import time
import os

graph = orm.ORM()
fd = open(os.getcwd() + "/tweet_output/output.txt", 'w+')

def readFile():
  with open(os.getcwd() + "/tweet_input/tweets.txt") as tweets:
    for line in tweets:

      # the file may contain messages related to rate limit and connection. Such
      # are captured in the exception and ignored
      try:
        tweet = json.loads(str(line.rstrip('\n')))
        #pprint.pprint(tweet)
        hashtags = [x['text'] for x in tweet['entities']['hashtags']]
        hashtags = list(set(hashtags)) # remove duplicates 
        createdAt = str(tweet['created_at'])
      except Exception as e:
        continue
      
      # get seconds since epoch time (1/1/1970). This is prepended to the hashtag
      # list and passed to the graph API
      createdAt = calendar.timegm(time.strptime(createdAt, '%a %b %d %H:%M:%S %z %Y'))
      
      hashtags.insert(0, int(createdAt))
      # adds hashtags from the current tweet to the graph. Returns the current
      # average vertex degree
      avg_degree = graph.addTweet(hashtags)
      fd.writelines(str.format("{0:.2f}\n", avg_degree))

    #graph.printPq()
    #graph.printGraph()

if __name__=="__main__":
  readFile()
  fd.close()
