/// @brief OPC UA Server main.
/// @license GNU LGPL
///
/// Distributed under the GNU LGPL License
/// (See accompanying file LICENSE or copy at
/// http://www.gnu.org/licenses/lgpl.html)
///
#include <iostream>
#include <sstream>
#include <algorithm>
#include <time.h>

#include <thread>
#include <chrono>

#include <opc/ua/node.h>
#include <opc/ua/subscription.h>
#include <opc/ua/server/server.h>




using namespace OpcUa;

class SubClient : public SubscriptionHandler
{
  void DataChange(uint32_t handle, const Node& node, const Variant& val, AttributeId attr) override
  {
    std::cout << "Received DataChange event for Node " << node << std::endl;
  }
};

class Diu
  {
    public:
       Diu(OpcUa::UaServer& server, uint32_t spaceidx, uint32_t id);
       void SetLocalValue(uint32_t value);
       void SetRemoteValue(float value);
       void TriggerEvent(Event event);
	   void Calibrate();

    protected:
		OpcUa::UaServer Server;
		uint32_t SpaceID;
		uint32_t ID;
      std::string Name;
	  Node DiuObject;
	  Node DeviceID;
	  Node LocalValue;
	  Node RemoteValue;
	  Node Correction;
	  Node CalibrateSignal;
	  Node CalibrateTime;
	  Node Temperature;
};


Diu::Diu(OpcUa::UaServer& server, uint32_t spaceid, uint32_t id)
{
	std::stringstream ss;
	Server = server;
	SpaceID = spaceid;
	ID = id;

	//Create our address space using different methods
	Node objects = server.GetObjectsNode();

	//Add a custom object with specific nodeid
	NodeId nid(id, spaceid);

	Name = "DIU";
	ss << id;
	Name += ss.str();
	QualifiedName qn(Name, SpaceID);
	DiuObject = objects.AddObject(nid, qn);

	//Add a variable and a property with auto-generated nodeid to our custom object
	DeviceID = DiuObject.AddVariable(SpaceID, "DIU ID", Variant(ID));
	LocalValue = DiuObject.AddVariable(SpaceID, "Local Value", Variant(8));
	RemoteValue = DiuObject.AddVariable(SpaceID, "Remote Value", Variant(7.8));
	Correction = DiuObject.AddVariable(SpaceID, "Correction", Variant(6.8));
	CalibrateSignal = DiuObject.AddVariable(SpaceID, "Calibrate Signal", Variant(5.8));
	CalibrateTime = DiuObject.AddVariable(SpaceID, "Calibrate Time", Variant(4.8));
	Temperature = DiuObject.AddVariable(SpaceID, "Temperature", Variant(3.8));
}

void Diu::SetLocalValue(uint32_t value)
{
	LocalValue.SetValue(Variant(value));
}

void Diu::SetRemoteValue(float value)
{
	RemoteValue.SetValue(Variant(value));
}
void Diu::TriggerEvent(Event event)
{

}
void Diu::Calibrate()
{

}


void RunServer()
{
  //First setup our server
  const bool debug = true;
  OpcUa::UaServer server(debug);
  server.SetEndpoint("opc.tcp://localhost:4841/diu/server");
  server.SetServerURI("urn://diuseserver.freeopcua.github.io");
  server.Start();

  //then register our server namespace and get its index in server
  uint32_t idx = server.RegisterNamespace("http://diu.freeopcua.github.io");


  Diu diu01 = Diu(server, idx, 1);
  Diu diu02 = Diu(server, idx, 2);
  Diu diu03 = Diu(server, idx, 3);
  Diu diu04 = Diu(server, idx, 4);

  #if 0
  //Create our address space using different methods
  Node objects = server.GetObjectsNode();

  //Add a custom object with specific nodeid
  NodeId nid(99, idx);
  QualifiedName qn("NewObject", idx);
  Node newobject = objects.AddObject(nid, qn);

  //Add a variable and a property with auto-generated nodeid to our custom object
  Node myvar = newobject.AddVariable(idx, "MyVariable", Variant(8));
  Node myprop = newobject.AddVariable(idx, "MyProperty", Variant(8.8));
#endif

  //browse root node on server side
  Node root = server.GetRootNode();
  std::cout << "Root node is: " << root << std::endl;
  std::cout << "Childs are: " << std::endl;
  for (Node node : root.GetChildren())
  {
    std::cout << "    " << node << std::endl;
  }


  //Uncomment following to subscribe to datachange events inside server
  /*
  SubClient clt;
  std::unique_ptr<Subscription> sub = server.CreateSubscription(100, clt);
  sub->SubscribeDataChange(myvar);
  */



  //Now write values to address space and send events so clients can have some fun
  uint32_t counter = 0;
  diu01.SetLocalValue(counter); //will change value and trigger datachange event

  //Create event
  server.EnableEventNotification();
  //Event ev(ObjectId::BaseEventType); //you should create your own type
  //ev.Severity = 2;
  //ev.SourceNode = ObjectId::Server;
  //ev.SourceName = "Event from FreeOpcUA";
  //ev.Time = DateTime::Current();


  std::cout << "Ctrl-C to exit" << std::endl;
  for (;;)
  {
    diu01.SetLocalValue(++counter); //will change value and trigger datachange event
    std::stringstream ss;
    ss << "This is event number: " << counter;
   // ev.Message = LocalizedText(ss.str());
   // server.TriggerEvent(ev);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  }

  server.Stop();
}

int main(int argc, char** argv)
{
  try
  {
    RunServer();
  }
  catch (const std::exception& exc)
  {
    std::cout << exc.what() << std::endl;
  }
  return 0;
}

