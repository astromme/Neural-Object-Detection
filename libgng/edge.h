
#ifndef _EDGE_H
#define _EDGE_H

namespace GNG {
  class Node;

  /**
      Edges in the GNG are undirected.  However for ease of
      implementation, the edges are represented as one-way. For example,
      if unitA and unitB and connected, then unitA maintains an edge to
      unitB and unitB maintains an edge to unitA.  Edges also maintain
      their age.  If an edge becomes too old, it will be removed.
  */
  class Edge {
    
    public:
      Edge(GNG::Node* from, GNG::Node* to);
      ~Edge();
      
      int id() const;
      
      GNG::Node* from() const;
      GNG::Node* to() const;
      
      void incrementAge();
      void setAge(int age);
      int age() const;
      void resetAge();
      
      int totalAge() const;
      void incrementTotalAge();
      
      // timestamp is in milliseconds since the gng was started
      int lastUpdated();
      void setLastUpdated(int time);
          
    private:
      const int m_id;
      GNG::Node *m_from;
      GNG::Node *m_to;
      int m_age;
      int m_totalAge;
      int m_lastUpdated;
  };

}

#endif // _EDGE_H
