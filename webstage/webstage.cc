#include <getopt.h>
#include <websim.hh>
#include <stage.hh>
using namespace Stg;
#include "config.h"

/* options descriptor */
static struct option longopts[] = {
	{ "gui",  no_argument,   NULL,  'g' },
	{ "port",  required_argument,   NULL,  'p' },
	{ "host",  required_argument,   NULL,  'h' },
	{ "federation",  required_argument,   NULL,  'f' },
	{ NULL, 0, NULL, 0 }
};

class WebStage : public websim::WebSim
{
  Stg::World* world;
  
public:
  WebStage( Stg::World* world,
				const std::string& host, const unsigned short port ) :
	 websim::WebSim( host, port ),
	 world(world)
  {
  }
  
  virtual ~WebStage()
  {}
  
  void Push( const std::string& name )
  {
	 Stg::Model* mod = world->GetModel( name.c_str() );
	 if( mod )
		{
		  websim::Pose p;
		  websim::Velocity v;
		  websim::Acceleration a;

		  Stg::Pose sp = mod->GetPose(); 
		  p.x = sp.x;
		  p.y = sp.y;
		  p.z = sp.z;
		  p.a = sp.a;
		  
		  Stg::Velocity sv = mod->GetVelocity(); 
		  v.x = sv.x;
		  v.y = sv.y;
		  v.z = sv.z;
		  v.a = sv.a;

		  SetPuppetPVA( name, p, v, a );
		}
	 else
		printf( "Warning: attempt to push PVA for unrecognized model \"%s\"\n",
				  name.c_str() );
  }
  
  void Push()
  {
	 for( std::map<std::string,Puppet*>::iterator it = puppets.begin();
			it != puppets.end();
			it++ )
		{
		  Puppet* pup = it->second;
		  assert(pup);
		  
		  Stg::Model* mod = world->GetModel( pup->name.c_str() );
		  assert(mod);
		  
		  websim::Pose p;
		  websim::Velocity v;
		  websim::Acceleration a;
		  
		  Stg::Pose sp = mod->GetPose(); 
		  p.x = sp.x;
		  p.y = sp.y;
		  p.z = sp.z;
		  p.a = sp.a;
		  
		  Stg::Velocity sv = mod->GetVelocity(); 
		  v.x = sv.x;
		  v.y = sv.y;
		  v.z = sv.z;
		  v.a = sv.a;
		  
		  pup->Push( p,v,a );
		  printf( "pushing puppet %s\n", pup->name.c_str() );
		}
  }


  // Interface to be implemented by simulators
  virtual bool CreateModel(const std::string& name, 
									const std::string& type,
									std::string& error)
  { 
	 printf( "create model name:%s type:%s\n", name.c_str(), type.c_str() ); 
	 
	 Model* mod = world->CreateModel( NULL, type.c_str() ); // top level models only for now
	 
	 // rename the model and store it by the new name
	 mod->SetToken( name.c_str() );
	 world->AddModel( mod );
	 

	 printf( "done." );
	 return true;
  }

  virtual bool DeleteModel(const std::string& name,
									std::string& error)
  {
	 printf( "delete model name:%s \n", name.c_str() ); 
	 return true;
  }

  virtual bool SetModelPVA(const std::string& name, 
									const websim::Pose& p,
									const websim::Velocity& v,
									const websim::Acceleration& a,
									std::string& error)
  {
	 //printf( "set model PVA name:%s\n", name.c_str() ); 	 

	 Model* mod = world->GetModel( name.c_str() );
	 if( mod )
		{
		  mod->SetPose( Stg::Pose( p.x, p.y, p.z, p.a ));
		  mod->SetVelocity( Stg::Velocity( v.x, v.y, v.z, v.a ));		 
		  // stage doesn't model acceleration
		}
	 else
		printf( "Warning: attempt to set PVA for unrecognized model \"%s\"\n",
				  name.c_str() );

	 return true;
  }

  virtual bool GetModelPVA(const std::string& name, 
									websim::Time& t,
									websim::Pose& p,
									websim::Velocity& v,
									websim::Acceleration& a,
									std::string& error)
  {
	 //printf( "get model name:%s\n", name.c_str() ); 

	 t = GetTime();

	 Model* mod = world->GetModel( name.c_str() );
	 if( mod )
		{
		  Stg::Pose sp = mod->GetPose(); 
		  p.x = sp.x;
		  p.y = sp.y;
		  p.z = sp.z;
		  p.a = sp.a;

		  Stg::Velocity sv = mod->GetVelocity(); 
		  v.x = sv.x;
		  v.y = sv.y;
		  v.z = sv.z;
		  v.a = sv.a;
		}
	 else
		printf( "Warning: attempt to set PVA for unrecognized model \"%s\"\n",
				  name.c_str() );

	 return true;
  }

  virtual bool GetLaserData(const std::string& name,			
  websim::Time& t,												
  uint32_t& resolution,
  double& fov,
  websim::Pose& p,
  std::vector<double>& ranges,													
  std::string& error){


	 t = GetTime();

	 Model* mod = world->GetModel( name.c_str() );
	 if( mod )
		{
                     ModelLaser* laser = (ModelLaser*)mod->GetModel("laser:0");  		
                     
		     if(laser){
		     		uint32_t sample_count=0;
  		     		stg_laser_sample_t* scan = laser->GetSamples( &sample_count );
        	     		assert(scan);
		     
				stg_laser_cfg_t cfg = laser->GetConfig();
		     		resolution =  cfg.resolution;
				fov = cfg.fov;
               	     		
				for(unsigned int i=0;i<sample_count;i++)
					ranges.push_back(scan[i].range);
		     }else{

				printf( "Warning: attempt to get laser data for unrecognized laser model of model \"%s\"\n",
				  name.c_str() );
  				return false;


		          }
	         	  
		}
	 else{
		printf( "Warning: attempt to get laser data for unrecognized model \"%s\"\n",
				  name.c_str() );
  		return false;
	     }

  return true;

  }					   
  virtual bool GetLaserCfgData(const std::string& name,
									websim::Time& t,
									uint32_t& resolution,
									double& fov,
						 			websim::Pose& p,
        								std::string& error)
{

	 t = GetTime();

	 Model* mod = world->GetModel( name.c_str() );
	 if( mod )
		{
                     ModelLaser* laser = (ModelLaser*)mod->GetModel("laser:0");  		
                     
		     if(laser){
				stg_laser_cfg_t cfg = laser->GetConfig();
		     		resolution =  cfg.resolution;
				fov = cfg.fov;
				//There is no way to access the position of the laser
		     }else{

				printf( "Warning: attempt to get laser config data for unrecognized laser model of model \"%s\"\n",
				  name.c_str() );
  				return false;


		          }
	         	  
		}
	 else{
		printf( "Warning: attempt to get laser config data for unrecognized model \"%s\"\n",
				  name.c_str() );
  		return false;
	     }

  return true;

  }					   
  virtual bool GetRangerData(const std::string& name,
									websim::Time& t,
									std::vector<double>& ranges,									    std::string& response){
	 t = GetTime();

	 Model* mod = world->GetModel( name.c_str() );
	 if( mod )
		{
                     ModelRanger* ranger = (ModelRanger*)mod->GetModel("ranger:0");  		
                     
		     if(ranger){
					uint32_t count = ranger->sensor_count;
					if(ranger->samples)
						for(unsigned int i=0;i<count;i++)
							ranges.push_back(ranger->samples[i]);
					//std::copy(ranger->samples,ranger->samples+ranger->sensor_count,ranges.begin());
				
		     }else{

				printf( "Warning: attempt to get ranger data for unrecognized ranger model of model \"%s\"\n",
				  name.c_str() );
  				return false;


		          }
	         	  
		}
	 else{
		printf( "Warning: attempt to get ranger data for unrecognized model \"%s\"\n",
				  name.c_str() );
  		return false;
	     }

  return true;

}

   virtual bool GetRangerCfgData(const std::string& name,
									websim::Time& t,
									std::vector<websim::Pose>& p,
									std::string& response)
   {

	
	 t = GetTime();

	 Model* mod = world->GetModel( name.c_str() );
	 if( mod )
		{
                     ModelRanger* ranger = (ModelRanger*)mod->GetModel("ranger:0");  		
                     
		     if(ranger){
					uint32_t count = ranger->sensor_count;
					if(ranger->sensors)
						for(unsigned int i=0;i<count;i++){
							websim::Pose pos;
							Pose rpos;
							rpos = ranger->sensors[i].pose;
							pos.x = rpos.x;
							pos.y = rpos.y;
							pos.z = rpos.z;
							pos.a = rpos.a;
							p.push_back(pos);					
						}
				
		     }else{

				printf( "Warning: attempt to get ranger Cfg data for unrecognized ranger model of model \"%s\"\n",
				  name.c_str() );
  				return false;


		          }
	         	  
		}
	 else{
		printf( "Warning: attempt to get ranger Cfg data for unrecognized model \"%s\"\n",
				  name.c_str() );
  		return false;
	     }

  	return true;


   }

  virtual websim::Time GetTime()
  {
	 stg_usec_t stgtime = world->SimTimeNow();

	 websim::Time t;
	 t.sec = stgtime / 1e6;
	 t.usec = stgtime - (t.sec * 1e6);
	 return t;
  }

//   static void UpdatePuppetCb( const std::string& name, WebSim::Puppet* pup, void* arg )
//   {
// 	 WebStage* ws = (WebStage*)arg;
// 	 ws->Push( pup->name );
//   }

};


int main( int argc, char** argv )
{
  // initialize libstage - call this first
  Stg::Init( &argc, &argv );

  printf( "WebStage built on %s %s\n", PROJECT, VERSION );
  
  std::string fedfilename = "";
  std::string host = "localhost";
  unsigned short port = 8000;
  
  int ch=0, optindex=0;
  bool usegui = true;
  bool usefedfile = false;

  while ((ch = getopt_long(argc, argv, "gh:p:f:", longopts, &optindex)) != -1)
	 {
		switch( ch )
		  {
		  case 0: // long option given
			 printf( "option %s given", longopts[optindex].name );
			 break;
		  case 'g': 
			 usegui = false;
			 printf( "[GUI disabled]" );
			 break;
		  case 'p':
			 port = atoi(optarg);
			 break;
		  case 'h':
			 host = optarg;
			 break;
		  case 'f':
			 fedfilename = optarg;
			 usefedfile = true;
			 break;
		  case '?':  
			 break;
		  default:
			 printf("unhandled option %c\n", ch );
		  }
	 }
  
  puts("");// end the first start-up line

  const char* worldfilename = argv[optind];

  printf( "[webstage] %s:%u fed %s world %s\n",
			 host.c_str(), 
			 port, 
			 fedfilename.c_str(), 
			 worldfilename );
			 
  World* world = ( usegui ? 
						 new WorldGui( 400, 300, worldfilename ) : 
						 new World( worldfilename ) );
  world->Load( worldfilename );

  WebStage ws( world, host, port );
  
  if( usefedfile )
	 ws.LoadFederationFile( fedfilename );

  puts( "entering main loop" );

  //close program once time has completed
  bool quit = false;
  while( ! quit )
	 {
		// todo? check for changes?
		// send my updates		
		ws.Push();
		//puts( "pushes  done" );

		// tell my friends to start simulating
		ws.Go();
		
		// puts( "go done" );
		
		// update Stage
		world->Update();
		//puts( "update done" );

		// wait for goes from all my friends
		ws.Wait();			
		//puts( "wait done" );
	 }

  printf( "Webstage done.\n" );
  return 0;			
}