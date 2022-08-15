#include "token/token.h"
#include "json/json.h"

int main() {

  // Test to convert JSON to CPP object

  jsonObject object = new jsonObject(
      {"employee":
        { "name" : "Rohan",
          "salary" : 69420,
          "married" : false
        }
      }
    );

    //Expected output is Rohan
    printf(object.name);
    printf(object.salary * 12);

}
