#include<iostream>

using namespace std;
int main() {
	setvbuf(stdin, NULL, _IOFBF, 30);
  	setvbuf(stdout, NULL, _IONBF, 0);
    string username, password;
	cout<<"Please enter your name: "; //24 chars
	cin>>username;
	cout<<"Please enter your password: "; //28 chars
	cin>>password;


    return 0;
}
