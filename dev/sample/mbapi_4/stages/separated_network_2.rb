system( "cd ../../../" )

def start_app( servers, clients, stagepoints, endpoints )

	args = []
	if not servers.empty?
		args << servers.map{|v| "-s #{v}"}.join(" ")
	end
	if not clients.empty?
		args << clients.map{|v| "-c #{v}"}.join(" ")
	end
	if not stagepoints.empty?
		args << stagepoints.map{|v| "--stagepoint #{v}"}.join(" ")
	end

	if not endpoints.empty?
		args << endpoints.map{|v| "--endpoint \"#{v[0]}\" #{v[1]} "}.join(" ")
	end

	cmd = "start ../../../sample.mbapi_4.stages.exe #{args.join(" ")}"
	puts cmd
	system( cmd )
	sleep( 1 )
end


start_app(
	[],
	[":55050", ":55051", ":55052", ":55053", ":55054", ":55055", ":55056"],
	[],
	[] )

start_app(
	[":55050"],
	[],
	["stage1@ep1"],
	[] )

start_app(
	[":55051"],
	[],
	["stage2@ep1"],
	[] )

start_app(
	[":55052"],
	[],
	["stage3@ep1"],
	[] )

start_app(
	[":55053"],
	[],
	["stage1@ep2"],
	[] )

start_app(
	[":55054"],
	[],
	["stage2@ep2"],
	[] )

start_app(
	[":55055"],
	[],
	[],
	[["ep1[stage1,stage2,stage3]", "ep2" ] ] )

start_app(
	[":55056"],
	[],
	[],
	[["ep2[stage1,stage2]", "ep1" ] ] )
