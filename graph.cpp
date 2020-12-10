#include "graph.h"

const Vertex Graph::InvalidVertex = Vertex(-1);
const int Graph::InvalidWeight = INT_MIN;
const string Graph:: InvalidLabel = "_CS225INVALIDLABEL";
const Edge Graph::InvalidEdge = Edge(Graph::InvalidVertex, Graph::InvalidVertex, Graph::InvalidWeight, Graph::InvalidLabel);

Graph::Graph(string connections_file, string vertices_file, bool weighted) 
    : weighted(weighted), directed(false), random(Random(0)) {
    
    ifstream connections;
    ifstream vertices_stream;
    connections.open(connections_file);
    vertices_stream.open(vertices_file);

    string line;
    vector<Vertex> vertex_v;
    string delim = ",";
    while (getline(vertices_stream, line)) {
        string index = line.substr(0, line.find(delim));
        std::stringstream idx(index);
        int i;
        idx >> i;

        string x_coor = line.substr(1, line.find(delim));
        std::stringstream x_str(x_coor);
        double x;
        x_str >> x;

        string y_coor = line.substr(2, line.find(delim));
        std::stringstream y_str(y_coor);
        double y;
        y_str >> y;

        insertVertex(Vertex(i, x, y));
        vertex_v.push_back(Vertex(i, x, y));
    }

    string n;
    while (getline(connections, n)) {
        string index = n.substr(0, n.find(delim));
        std::stringstream idx(index);
        int i;
        idx >> i;

        string vertex1 = n.substr(1, n.find(delim));
        std::stringstream ver1(vertex1);
        int v1;
        ver1 >> v1;
        
        string vertex2 = n.substr(2, n.find(delim));
        std::stringstream ver2(vertex2);
        int v2;
        ver2 >> v2;

        string weight = n.substr(3, n.find(delim));
        std::stringstream we(weight);
        int w;
        we >> w;

        Vertex start;
        Vertex end;
        for(Vertex vx : vertex_v) {
            if (vx.getIndex() == v1) {
                start = vx;
            }
            if (vx.getIndex() == v2) {
                end = vx;
            }
        }

        insertEdge(start, end);
        if (weighted) setEdgeWeight(start, end, w);
    }
}

Graph::Graph(bool weighted) : weighted(weighted),directed(false),random(Random(0))
{
}

Graph::Graph(bool weighted, bool directed) : weighted(weighted),directed(directed),random(Random(0))
{
}

Graph::Graph(bool weighted, int numVertices, unsigned long seed)
    :weighted(weighted),
      directed(false),
     random(Random(seed)) 
{
    if (numVertices < 2)
    {
     error("numVertices too low");
     exit(1);
    }

    vector<Vertex> vertices;
    for (int i = 0; i < numVertices; i++)
    {
        insertVertex(i);
        vertices.push_back(i);
    }

    // make sure all vertices are connected
    random.shuffle(vertices);
    Vertex cur = vertices[0];
    for (size_t i = 0; i < vertices.size() - 1; ++i)
    {
        Vertex next = vertices[i + 1];
        insertEdge(cur, next);
        if (weighted) 
        {
            int weight = random.nextInt();
            setEdgeWeight(cur, next, weight);
        }
        cur = next;
    }

    // keep the graph from being overpopulated with edges,
    //  while still maintaining a little randomness
    int numFailures = 0;
    int idx = 0;
    random.shuffle(vertices);
    while (numFailures < 2) 
    {
        if (!insertEdge(vertices[idx], vertices[idx + 1])) 
        {
            ++numFailures;
        } 
        else 
        {
            // if insertEdge() succeeded...
            if (weighted)
                setEdgeWeight(vertices[idx], vertices[idx + 1],
                              random.nextInt());
            ++idx;
            if (idx >= numVertices - 2) 
            {
                idx = 0;
                random.shuffle(vertices);
            }
        }
    }
}

vector<Vertex> Graph::getAdjacent(Vertex source) const 
{
    auto lookup = adjacency_list.find(source);

    if(lookup == adjacency_list.end())
        return vector<Vertex>();
	
     else
    {
        vector<Vertex> vertex_list;
        const unordered_map <Vertex, Edge> & map = adjacency_list[source];
        for (auto it = map.begin(); it != map.end(); it++)
        {
            vertex_list.push_back(it->first);
        }
        return vertex_list;
    }
}


Vertex Graph::getStartingVertex() const
{
    return adjacency_list.begin()->first;
}

vector<Vertex> Graph::getVertices() const
{
    vector<Vertex> ret;

    for(auto it = adjacency_list.begin(); it != adjacency_list.end(); it++)
    {
        ret.push_back(it->first);
    }

    return ret;
}

Edge Graph::getEdge(Vertex source , Vertex destination) const
{
    if(assertEdgeExists(source, destination, __func__) == false)
        return Edge();
    Edge ret = adjacency_list[source][destination];
    return ret;
}

vector<Edge> Graph::getEdges() const
{
    if (adjacency_list.empty())
        return vector<Edge>();

    vector<Edge> ret;
    set<pair<Vertex, Vertex>> seen;

    for (auto it = adjacency_list.begin(); it != adjacency_list.end(); it++)
    {
        Vertex source = it->first;
        for (auto its = adjacency_list[source].begin(); its != adjacency_list[source].end(); its++)
        {
            Vertex destination = its->first;
            if(seen.find(make_pair(source, destination)) == seen.end())
            {
                //this pair is never added to seen
                ret.push_back(its->second);
                seen.insert(make_pair(source,destination));
                if(!directed)
                {
                    seen.insert(make_pair(destination, source));
                }
            }
        }
    }

    return ret;
}

bool Graph::vertexExists(Vertex v) const
{
    return assertVertexExists(v, "");
}

bool Graph::edgeExists(Vertex source, Vertex destination) const
{
    return assertEdgeExists(source, destination, "");
}

Edge Graph::setEdgeLabel(Vertex source, Vertex destination, string label)
{
    if (assertEdgeExists(source, destination, __func__) == false)
        return InvalidEdge;
    Edge e = adjacency_list[source][destination];
    Edge new_edge(source, destination, e.getWeight(), label);
    adjacency_list[source][destination] = new_edge;

    if(!directed)
    {
        Edge new_edge_reverse(destination,source, e.getWeight(), label);
        adjacency_list[destination][source] = new_edge_reverse;
    }
    return new_edge;
}


string Graph::getEdgeLabel(Vertex source, Vertex destination) const
{
    if(assertEdgeExists(source, destination, __func__) == false)
        return InvalidLabel;
    return adjacency_list[source][destination].getLabel();
}

int Graph::getEdgeWeight(Vertex source, Vertex destination) const
{
    if (!weighted)
        error("can't get edge weights on non-weighted graphs!");

    if(assertEdgeExists(source, destination, __func__) == false)
        return InvalidWeight;
    return adjacency_list[source][destination].getWeight();
}

void Graph::insertVertex(Vertex v)
{
    // will overwrite if old stuff was there
    removeVertex(v);
    // make it empty again
    adjacency_list[v] = unordered_map<Vertex, Edge>();
}


Vertex Graph::removeVertex(Vertex v)
{

    if (adjacency_list.find(v) != adjacency_list.end())
    {
        if(!directed){
            for (auto it = adjacency_list[v].begin(); it != adjacency_list[v].end(); it++)
            {
                Vertex u = it->first;
                adjacency_list[u].erase(v); 
            }
            adjacency_list.erase(v);
            return v;
        }
        
        adjacency_list.erase(v);
        for(auto it2 = adjacency_list.begin(); it2 != adjacency_list.end(); it2++)
        {
            Vertex u = it2->first;
            if (it2->second.find(v)!=it2->second.end())
            {
                it2->second.erase(v);
            }
        }
        return v;
    }

    return InvalidVertex;
}

bool Graph::insertEdge(Vertex source, Vertex destination)
{
    if(adjacency_list.find(source)!= adjacency_list.end() 
    && adjacency_list[source].find(destination)!= adjacency_list[source].end())
    {
        //edge already exit
        return false;
    }

    if(adjacency_list.find(source)==adjacency_list.end())
    {
        adjacency_list[source] = unordered_map<Vertex, Edge>();
    }
        //source vertex exists
    adjacency_list[source][destination] = Edge(source, destination);
    if(!directed)
    {
        if(adjacency_list.find(destination)== adjacency_list.end())
        {
            adjacency_list[destination] = unordered_map<Vertex, Edge>();
        }
        adjacency_list[destination][source] = Edge(source, destination);
    }
    
    return true;
}

Edge Graph::removeEdge(Vertex source, Vertex destination)
{
    if(assertEdgeExists(source, destination, __func__) == false)
        return InvalidEdge;
    Edge e = adjacency_list[source][destination];
    adjacency_list[source].erase(destination);
    // if undirected, remove the corresponding edge
    if(!directed)
    {
        adjacency_list[destination].erase(source);
    }
    return e;
}


Edge Graph::setEdgeWeight(Vertex source, Vertex destination, double weight)
{
    if (assertEdgeExists(source, destination, __func__) == false)
        return InvalidEdge;
    Edge e = adjacency_list[source][destination];
    //std::cout << "setting weight: " << weight << std::endl;
    Edge new_edge(source, destination, weight, e.getLabel());
    adjacency_list[source][destination] = new_edge;

    if(!directed)
        {
            Edge new_edge_reverse(destination,source, weight, e.getLabel());
            adjacency_list[destination][source] = new_edge_reverse;
        }

    return new_edge;
}

bool Graph::assertVertexExists(Vertex v, string functionName) const
{
    if (adjacency_list.find(v) == adjacency_list.end())
    {
        if (functionName != "")
            error(functionName + " called on nonexistent vertices");
        return false;
    }
    return true;
}

bool Graph::assertEdgeExists(Vertex source, Vertex destination, string functionName) const
{
    if(assertVertexExists(source,functionName) == false)
        return false;
    if(adjacency_list[source].find(destination)== adjacency_list[source].end())
    {
        if (functionName != "")
            error(functionName + " called on nonexistent edge " + to_string(source.getIndex()) + " -> " + to_string(destination.getIndex()));
        return false;
    }

    if(!directed)
    {
        if (assertVertexExists(destination,functionName) == false)
            return false;
        if(adjacency_list[destination].find(source)== adjacency_list[destination].end())
        {
            if (functionName != "")
                error(functionName + " called on nonexistent edge " + to_string(destination.getIndex()) + " -> " + to_string(source.getIndex()));
            return false;
        }
    }
    return true;
}

bool Graph::isDirected() const
{
    return directed;
}

void Graph::clear()
{
    adjacency_list.clear();
}


/**
 * Prints a graph error and quits the program.
 * The program is exited with a segfault to provide a stack trace.
 * @param message - the error message that is printed
 */
void Graph::error(string message) const
{
    cerr << "\033[1;31m[Graph Error]\033[0m " + message << endl;
}

/**
 * Creates a name for snapshots of the graph.
 * @param title - the name to save the snapshots as
 */
void Graph::initSnapshot(string title)
{
    picNum = 0;
    picName = title;
}

/**
 * Saves a snapshot of the graph to file.
 * initSnapshot() must be run first.
 */
void Graph::snapshot()
{
    std::stringstream ss;
    ss << picNum;
    string newName = picName + ss.str();
    savePNG(newName);
    ++picNum;
}

/**
 * Prints the graph to stdout.
 */
void Graph::print() const
{
    for (auto it = adjacency_list.begin(); it != adjacency_list.end(); ++it) 
    {
        cout << it->first << endl;
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) 
        {
            std::stringstream ss;
            ss << it2->first; 
            string vertexColumn = "    => " + ss.str();
            vertexColumn += " " ;
            cout << std::left << std::setw(26) << vertexColumn;
            string edgeColumn = "edge label = \"" + it2->second.getLabel()+ "\"";
            cout << std::left << std::setw(26) << edgeColumn;
            if (weighted)
                cout << "weight = " << it2->second.getWeight();
            cout << endl;
        }
        cout << endl;
    }
}

/** 
 * Render graph onto png of map
 */
void Graph::render(Graph g, cs225::PNG png) const {
    // go through vertices and edges of graph
    // edit png to one color if it's a vertex and another if its an edge
    // for every vertice, set the pixel at that x and y to be a color
    // for every edge, not sure yet but perhaps get the weight and then change x and y for that length (for loop ?) to different color
        // find slope of edges to get x distance and y distance

    // get vector of adjacent vertices, find geometric distance between source vertex
        // and each of its adjacent ones
    // get adjacent vertices for each v, find distance between them, 
        // color each pixel along edge
    vector<Vertex> vertices = g.getVertices();
    //vector<Edge> edges = g.getEdges();
    double dx = 0.0;
    double dy = 0.0;
    double y;
    for (Vertex v : vertices) {
        // get the x and y
        double x_coor = v.getX();
        double y_coor = v.getY();
        // set the color of the correlating pixel (of the png)
        cs225::HSLAPixel& pixel = png.getPixel(x_coor, y_coor);
        pixel.h = 156;
        pixel.s = 1;
        pixel.l = 0.22;
        pixel.a = 1;
        vector<Vertex> adjacent = g.getAdjacent(v);
        for (Vertex a : adjacent) {
            //dist = sqrt((pow(v.getX() - a.getX(), 2)) + (pow(v.getY() - a.getY(), 2)));
            // compute slope
            dx = v.getX() - a.getX();
            dy = v.getY() - a.getY();
            for (double x = a.getX(); x <= v.getX(); x++) {
                y = a.getY() + dy * (x - a.getX()) / dx;
                // access every y associated with every x to get every pixel
                cs225::HSLAPixel& pixel = png.getPixel(x, y);
                // change color
                pixel.h = 0;
                pixel.s = 1;
                pixel.l = 0.32;
                pixel.a = 1;
            }
        }
    }
}

/**
 * Saves the graph as a PNG image.
 * @param title - the filename of the PNG image
 */
void Graph::savePNG(string title) const
{
    /* std::ofstream neatoFile;
    string filename = "images/" + title + ".dot";
    neatoFile.open(filename.c_str());

    if (!neatoFile.good())
        error("couldn't create " + filename + ".dot");

    neatoFile
        << "strict graph G {\n"
        << "\toverlap=\"false\";\n"
        << "\tdpi=\"1300\";\n"
        << "\tsep=\"1.5\";\n"
        << "\tnode [fixedsize=\"true\", shape=\"circle\", fontsize=\"7.0\"];\n"
        << "\tedge [penwidth=\"1.5\", fontsize=\"7.0\"];\n";

    vector<Vertex> allv = getVertices();
    //lambda expression
    sort(allv.begin(), allv.end(), [](const Vertex &lhs, const Vertex &rhs) {
        return lhs.getIndex() > rhs.getIndex();
    });

    int xpos1 = 100;
    int xpos2 = 100;
    int xpos, ypos;
    for (auto it : allv)
    {
        Vertex current = it;
        neatoFile
            << "\t\""
            << current
            << "\"";
        if (current[1] == '1')
        {
            ypos = 100;
            xpos = xpos1;
            xpos1 += 100;
        }
        else
        {
            ypos = 200;
            xpos = xpos2;
            xpos2 += 100;
        }
        neatoFile << "[pos=\"" << xpos << "," << ypos << "\"]";
        neatoFile << ";\n";
    }

    neatoFile << "\tedge [penwidth=\"1.5\", fontsize=\"7.0\"];\n";

    for (auto it = adjacency_list.begin(); it != adjacency_list.end(); ++it)
    {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            string vertex1Text = it->first;
            string vertex2Text = it2->first;

            neatoFile << "\t\"";
            neatoFile << vertex1Text;
            neatoFile << "\" -- \"";
            neatoFile << vertex2Text;
            neatoFile << "\"";

            string edgeLabel = it2->second.getLabel();
            if (edgeLabel == "WIN")
            {
                neatoFile << "[color=\"blue\"]";
            }
            else if (edgeLabel == "LOSE")
            {
                neatoFile << "[color=\"red\"]";
            }
            else
            {
                neatoFile << "[color=\"grey\"]";
            }
            if (weighted && it2->second.getWeight() != -1)
                neatoFile << "[label=\"" << it2->second.getWeight() << "\"]";

            neatoFile << "[constraint = \"false\"]"
                      << ";\n";
        }
    }

    neatoFile << "}";
    neatoFile.close();
    string command = "neato -n -Tpng " + filename + " -o " + "images/" + title + ".png 2> /dev/null";
    int result = system(command.c_str());

    if (result == 0)
    {
        cout << "Output graph saved as images/" << title << ".png" << endl;
    }
    else
    {
        cout << "Failed to generate visual output graph using `neato`. Install `graphviz` or `neato` to generate a visual graph." << endl;
    }

    string rmCommand = "rm -f " + filename + " 2> /dev/null";
    system(rmCommand.c_str());
    */
}
