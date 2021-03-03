#include<CRoom.h>

using namespace std;

int main()
{
	CRoom room1;
	++room1;
	++room1;
	++room1;
	++room1;
	--room1;

	cout << "Members: " << room1.getNumOfMembers() << endl;

	return 0;
}
