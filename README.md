so what this does is,
if you are in location1 and you send a message to location2,
it will save that message in a globalBuffer,
and every 20 seconds it will forward that globalBuffer to all machines,
it will decrement the TTL every 20 seconds until TTL == 0,

this program does not handle duplicates
