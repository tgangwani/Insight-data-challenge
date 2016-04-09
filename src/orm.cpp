// Author - Tanmay Gangwani

#include "boost/python.hpp"
#include "boost/python/raw_function.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/graph_traits.hpp"
#include <iostream>
#include <unordered_map>
#include <queue>
#include <set>
#include <algorithm>
#include <assert.h>  
#include "tweet.h"

using namespace boost;

#define cutoff mst-60

typedef adjacency_list<vecS, vecS, undirectedS> hashtagGraph;
typedef graph_traits<hashtagGraph>::vertex_descriptor vertex_t;
typedef std::unordered_map<std::string, vertex_t> vertex_map;
typedef graph_traits<hashtagGraph>::edge_iterator edge_iterator;

class ORM
{
  private:
    std::priority_queue<tweet, std::vector<tweet>, orderTweets> pq; // priority queue of tweets. Most recent tweet at the bottom of the queue 
    hashtagGraph g; // hashtag graph, stored as an adjacency list
    vertex_map m; // map from labels(hashtags) to vertex descriptors in the graph
    int64_t n_vertices;  // number of vertices in the graph
    int64_t sum; // sum of degree of all vertices in graph
    uint64_t mst; // most recent timestamp

  public:

    // constructor
    ORM(): pq(orderTweets()), mst(0), sum(0), n_vertices(0) {
    }

    // called from python to add a tweet
    double addTweet(python::list& ns) {
      
      #ifdef DEBUG
       std::cout<<"Incoming Tweet\n";
      #endif
  
      uint64_t creation_time = static_cast<uint64_t>(python::extract<uint64_t>(ns[0]));
      
      // check if we got the most recent tweet. If yes, we might need to remove some tweets
      if (creation_time > mst) {
        mst = creation_time;

        // go over the pq to check the tweets which need to be removed. Since
        // the stalest tweets sits at the top, this is a O(1) operation
        while(pq.size()) {
          const tweet& tw = pq.top();
          if(tw.creation_time < cutoff) {
            removeFromGraph(tw);
            pq.pop();
          }
          else {
            // stop checking since it is priority queue (i.e. ordered)
            break;
          }
        }
      }
      // if the tweet falls out of the window (_mst - 60), then we don't add this to the priority queue (and graph)
      else if(creation_time < cutoff) {
        #ifdef DEBUG
          std::cout<<"Creation time("<<creation_time<<") is less than the cutoff("<<cutoff<<"). Not adding tweet to graph.\n";
        #endif
        double avg_degree = n_vertices ? sum/static_cast<double>(n_vertices) : 0;
        return avg_degree;
      }                                     

      // if the number of hashtags in a tweet are less than 2, then we don't add this to the priority queue (and graph). We check for length 3 since the first arg. is timestamp
      if(len(ns) < 3) {
        #ifdef DEBUG
          std::cout<<"Number of hashtags in tweet is " << len(ns)-1 << ". Not adding tweet to graph.\n";
        #endif

        // even though this tweet is not added to the pq (graph), it may have removed some edges/nodes. We return the updated degree
        double avg_degree = n_vertices ? sum/static_cast<double>(n_vertices) : 0;
        return avg_degree;
      }

      std::list<std::string> l;
      for (int i = 1; i < len(ns); ++i) {
        l.emplace_back(static_cast<std::string>(python::extract<std::string>(ns[i])));
      }

      // create the tweet (structure) and add to priority queue (and graph)
      tweet tw(l, creation_time);
      pq.push(tw);
      addToGraph(tw);

      double avg_degree = n_vertices ? sum/static_cast<double>(n_vertices) : 0;
      return avg_degree;
    }
    
    // for each hashtag pair, remove edge
    void removeEdge(const std::string& s1, const std::string& s2) {

       // get the vertex descriptor for these hashtags from the vertex_map. These should definately exist!
       vertex_map::iterator it1 = m.find(s1);
       assert(it1 != m.end()); 
  
       vertex_map::iterator it2 = m.find(s2);
       assert(it2 != m.end()); 
  
       // get the edge between vertices. There may be multiple edges between them. This function returns just one edge
       // e is of type <edge_descriptor, bool> where the second arg. is true is an edge was found
       auto e = edge(it1->second, it2->second, g);   
       assert(e.second == true);
    
       #ifdef DEBUG
        std::cout << "Removing edge between " << it1->second << " and " << it2->second << "\n"; 
       #endif        
       
       // remove edge using the edge descriptor
       remove_edge(e.first, g);

       // if this was the last edge between these two vertices, then decrement the sum which is used to calculate the avg. vertex degree for the incoming tweet
       e = edge(it1->second, it2->second, g);
       if(e.second == false) {
        sum -= 2;
        assert(sum >= 0);
       }

       // if either of the endpoints have lost all edges (i.e. are orphan/disconnected), we decrease the n_vertices counter
       if(degree(it1->second, g) == 0) {
         --n_vertices;
         assert(n_vertices >= 0);
       }
       if(degree(it2->second, g) == 0) {
         --n_vertices;
         assert(n_vertices >= 0);
       }

    }

    // remove edges and nodes (if required) from the hashtag graph since the
    // tweet has fallen out of window
    void removeFromGraph(const tweet& tw) {
       const std::list<std::string>& hashtags= tw.hashtags;
                                                                        
       // this doubly nested for loop is used to create (n-choose-2) combinations of hashtags
       for(std::list<std::string>::const_iterator h_it = hashtags.begin(), h_end = hashtags.end(); h_it != h_end; ++h_it) {
          std::list<std::string>::const_iterator h_jt = h_it;
          std::advance(h_jt, 1);

          for(; h_jt != h_end; ++h_jt) {
            removeEdge(*h_it, *h_jt);
          }
       }
    }

    // for each hashtag pair, add an edge
    void addEdge(const std::string& s1, const std::string& s2) {
       
       // duplicates are removed in the python processing stage
       assert(s1.compare(s2) != 0);

       vertex_t endpoints[] = {0, 0};
       vertex_t n;
       uint8_t pos = -1;

       for(auto& s : {s1, s2}) {
         pos++;

         // check if a node already exists for this hashtag. This is done by checking the vertex_map. This search is a constant time operation on average
         vertex_map::iterator it = m.find(s);                           

         // if this is the first time we are encountering this hashtag, add a new vertex to the graph; otherwise use the existing mapping
         if(it == m.end()) {
          n = add_vertex(g);
          m.insert(std::make_pair(s, n));
         }
         else {
          n = it->second;
         }

         endpoints[pos] = n;
       }

       #ifdef DEBUG
        std::cout << "Adding edge between " << endpoints[0] << " and " << endpoints[1] << "\n"; 
       #endif        

       // if either of the endpoints have 0 degree, this means that these are new vertices in the graph. We increment the n_vertices counter
       if(degree(endpoints[0], g) == 0) {
        ++n_vertices;
       }
       if(degree(endpoints[1], g) == 0) {
        ++n_vertices;
       }

       // if this is the first edge between these two vertices, then increment the sum which is used to calculate the avg. vertex degree for the incoming tweet
       auto e = edge(endpoints[0], endpoints[1], g);
       if(e.second == false) {
        sum += 2;
       }
       
       // add an edge using the vertex descriptors
       add_edge(endpoints[0], endpoints[1], g);    
    }

    // add edges and nodes (if required) from the input tweet to the hashtag graph
    void addToGraph(const tweet& tw) {
       const std::list<std::string>& hashtags= tw.hashtags;
                                                                        
       // this doubly nested for loop is used to create (n-choose-2) combinations of hashtags
       for(std::list<std::string>::const_iterator h_it = hashtags.begin(), h_end = hashtags.end(); h_it != h_end; ++h_it) {
          std::list<std::string>::const_iterator h_jt = h_it;
          std::advance(h_jt, 1);

          for(; h_jt != h_end; ++h_jt) {
            addEdge(*h_it, *h_jt);
          }
       }
    }

    // function to print the priority queue
    void printPq() {
      // priority queue doesn't allow iteration. So, we make a destructive copy
      std::priority_queue<tweet, std::vector<tweet>, orderTweets> pq_copy(pq);
      
      for(uint8_t i=0; i<pq.size(); i++) {
       const tweet& tw = pq_copy.top();
       tw.print();
       pq_copy.pop();
      }
    }
                                           
    // print degree of each vertex in the graph
    void printVertexDegree() {
      auto vi = vertices(g);
      for (auto next = vi.first; next != vi.second; ++next) {
        std::cout<<"Degree of vertex " << *next << " is " << degree(*next, g) <<"\n";
      } 
    }

    // function to print the hashtag graph (iterate over all the edges)
    void printGraph() {
      std::cout<<"Number of vertices in graph = " << num_vertices(g) <<"\n";
      std::cout<<"Number of edges in graph = " << num_edges(g) <<"\n";
       
      std::pair<edge_iterator, edge_iterator> ei = edges(g);
      for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        std::cout << "(" << source(*edge_iter, g) << ", " << target(*edge_iter, g) << ")\n";
      }
    }
};

BOOST_PYTHON_MODULE(orm)
{
    python::class_<ORM>("ORM")
    .def("addTweet", &ORM::addTweet)
    .def("printPq", &ORM::printPq)
    .def("printGraph", &ORM::printGraph)
    ;
}
