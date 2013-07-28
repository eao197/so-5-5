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
	[":55055"],
	[],
	["stage1@ep2", "stage2@ep1"],
	[["ep1[stage1,stage2,stage3]", "ep2" ] ] )

start_app(
	[],
	[":55055"],
	["stage1@ep1", "stage2@ep2", "stage3@ep1"],
	[["ep2[stage1,stage2]", "ep1" ] ] )
