myid = 99999;
dir = 0;
dist = 0;
move = 0;

function set_uid(x)
	myid = x;
end

function set_dir(d)
	dir = d;
end

function event_player_move(player)
	player_x = API_get_x(player);
	player_y = API_get_y(player);
	my_x = API_get_x(myid);
	my_y = API_get_y(myid);
	if (player_x == my_x) then
		if (player_y == my_y) then
			if (move == 0) then
				move = 1;
				API_SendMessage(myid, player, "HELLO");
				API_RunAway(player, myid);
			end
		end
	end
end

function say_good_bye(player)
	if (move == 1) then
		my_x = API_get_x(myid);
		my_y = API_get_y(myid);
		if (dir == 0) then
			--my_x = my_x - 1;
		elseif (dir == 1) then
			--my_x = my_x + 1;
		elseif (dir == 2) then
			--my_y = my_y - 1;
		elseif (dir == 3) then
			--my_y = my_y + 1;
		end

		API_SetPos(my_x, my_y, myid, player);
		dist = dist + 1;
		API_RunAway(player, myid);
		if (dist >= 3) then
			move = 0;
			dist = 0;
			API_SendMessage(myid, player, "BYE");
		end
	end
end