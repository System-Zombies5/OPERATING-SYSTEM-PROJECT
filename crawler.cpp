#include <bits/stdc++.h> // standard c++ library
#include <unordered_map>
#include <curl/curl.h> //for downloading webpages
#include <stdio.h>
#include <regex> //to handle regular expressions
#include <thread>
#include <mutex>  //to avoid race conditions
#include <chrono> //time-related functions
using namespace std;

mutex m;

// download html in form of file
void get_page(const char *url, const char *file_name)
{
  // initialize the curl Object
  CURL *easyhandle = curl_easy_init(); 
  // set the url which will be used for http request
  curl_easy_setopt(easyhandle, CURLOPT_URL, url); 
  // open file in write mode
  FILE *file = fopen(file_name, "w");             
  // specify file where response data will be written
  curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, file); 
  // perform actual http request
  curl_easy_perform(easyhandle);
  // clean up easyhandle object and frees any associated resource                        
  curl_easy_cleanup(easyhandle);   
  // close the file                      
  fclose(file);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

// Links extraction from HTML file
vector<string> extract_hyperlinks(string html_file_name)
{
  string html; // stroring the html file
  ifstream read; 
  // file opened
  read.open(html_file_name);

  // character by character reading of the html file
  while (!read.eof())
  {
    if (read.eof())
      break; // stops if end of the file is reached
    char a;
    read.get(a);
    html.push_back(a);
  }
  // file closed
  read.close();

  static const regex hl_regex("<a href=\"(.*?)\">", regex_constants::icase); // initializing a regular expression object h1_regex that finds all hyperlinks in html file,icase mean not case sensitive
  vector<string> links;                                                      // it will store all the links read from file
  
  // storing all the links to the above formed vector of strings
  copy(sregex_token_iterator(html.begin(), html.end(), hl_regex, 1), sregex_token_iterator(), back_inserter(links));
  return links; // returns the vector
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// authenticating the links
void cleanUp(vector<string> &all_links)
{

  vector<string> final_links; // it will store all the valid links               
  for (int i = 0; i < all_links.size(); i++) // cleaner
  {
    string one_link = all_links[i]; // pick one link
    string cleaned_link = "";       // it will store cleaned link
    for (int j = 0; j < one_link.length(); j++)
    {
      if (one_link[j] == ' ' || one_link[j] == 34)
        break;                             // breaks if empty space or double quotation is found
      cleaned_link.push_back(one_link[j]); // character by character concatenation to string
    }
    // url is added to final vector after gettig useful part and validation of the link
    if (regex_match(cleaned_link, regex("((http|https)://)(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)")))
    {
      final_links.push_back(cleaned_link);
    }
  }
  // clearing original vector
  all_links.clear(); 
  // replacing it with final links vector
  all_links = final_links; 
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

unordered_map<string, bool> visited; // used to detect multiple occurence of a url 

void dfs_crawler(const char *url, const char *file_path, int depth, int bot_id)
{
  if (depth == 4 || visited[url])
    return;       // if depth=4 or url is visited
  string s = url; // store url in string s
  visited[s] = 1; // mark current url as visited

  m.lock(); // using it for synchronization
  
  // Clock function to calculate time taken by the program to extract and fetch the url from html file
  clock_t startTime = clock(); 
  get_page(url, file_path); // get html of current url

  vector<string> allLinksData = extract_hyperlinks(file_path); // get all links
  cleanUp(allLinksData);                                       // clean all the links
  m.unlock();                                                  // unlocking the mutex lock

  clock_t stopTime = clock(); // clock stops
  float elapsedTime = (float)(stopTime - startTime) / CLOCKS_PER_SEC;
  // Printing the time taken to generate the particular bot_id url
  cout << "Time taken to generate thread " << bot_id << " is: " << elapsedTime << " seconds" << endl; 
  cout << "Thread_id: " << bot_id << "\tLink: " << url << endl << endl; // print the url and thread id


  // DFS function
  for (string i : allLinksData) 
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
    const char *c = i.c_str();

    if (visited[i] != 1) // if url is not visited then call the function
    { 
      dfs_crawler(c, file_path, depth + 1, bot_id);
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int main()
{
  const char *filename = "1.txt"; // file to store the html
  thread t1(dfs_crawler, "https://www.iitism.ac.in/", filename, 0, 1);
  cout << endl;
  thread t2(dfs_crawler, "https://en.wikipedia.org/wiki/Main_page", filename, 0, 2);
  cout << endl;
  thread t3(dfs_crawler, "https://codeforces.com/", filename, 0, 3);
  cout << endl;
  // function to make the main thread wait until the newly created thread has finished its execution,all the threads have completed their execution before the program terminates.
  t1.join(); 
  t2.join();s
  t3.join();

  return 0;
}
