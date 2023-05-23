DISCONNECT_TIMEOUT_MS = 3000
DISCONNECT_NOTIFY_START_MS = 1000


import ggpo_py
import pygame
import sys
from pygame import Vector2

class PlayerStatus:
    def __init__(self):
        self.p1_pos = Vector2(50, 50)
        self.p2_pos = Vector2(150, 150)


    def serialize_inputs(self, player_input:int) -> Vector2:
        out_vec = Vector2(0, 0)

        if player_input>>0&1:
            out_vec.x -= 50
        if player_input>>1&1:
            out_vec.x += 50
        if player_input>>2&1:
            out_vec.y -= 50
        if player_input>>3&1:
            out_vec.y += 50
        
        return out_vec


    def update(self, inputs):
        p1_inputs, p2_inputs = inputs

        p1_vec = self.serialize_inputs(p1_inputs)
        p2_vec = self.serialize_inputs(p2_inputs)

        self.p1_pos += p1_vec * 1/60
        self.p2_pos += p2_vec * 1/60


class Game:
    def __init__(self, window:pygame.Surface, game_title:str, localport:int, remote_host:str, remote_port:int):
        
        self.window = window
        self.game_state = PlayerStatus()
        self.running = False
        self.clock = pygame.time.Clock()

        self.net_session = ggpo_py.GGPOSession(game_title, localport, 2, self.game_state)
        self.remote_host = remote_host
        self.remote_port = remote_port
        self.player   = None
        self.opponent = None


    def cbk_advance_frame(self, all_inputs, disconnected_players):
        self.advance_frame(all_inputs)


    def cbk_on_event(self, event:ggpo_py.GGPOEvent):
        if event.code == ggpo_py.GGPOEventCode.GGPO_EVENTCODE_TIMESYNC:
            # Our client is too far ahead so we go to sleep...
            # We give this waiting time to GGPO to manage things it didn't have time for
            ms_per_frame = 1000/60
            ms_wait = ms_per_frame * event.frames_ahead

            self.net_session.idle(round(ms_wait))


    def advance_frame(self, inputs):
        self.game_state.update(inputs)
        self.net_session.advance_frame()


    def draw_current_frame(self):
        self.window.fill((0, 0, 0))
        pygame.draw.rect(self.window, (255, 0, 0), pygame.Rect(self.game_state.p1_pos, (10, 10)))
        pygame.draw.rect(self.window, (0, 255, 0), pygame.Rect(self.game_state.p2_pos, (10, 10)))
        pygame.display.flip()


    def _add_player(self, player):
        self.net_session.add_player(player)
        err = self.net_session.set_frame_delay(player, 6)
        if err != ggpo_py.GGPOErrorCode.GGPO_ERRORCODE_SUCCESS:
            raise Exception("Failed to initialize Player: {}".format(err))
        

    def _add_opponent(self, opponent):
        err = self.net_session.add_player(opponent)
        if err != ggpo_py.GGPOErrorCode.GGPO_ERRORCODE_SUCCESS:
            raise Exception("Failed to initialize Player: {}".format(err))


    def init_network(self, own_player_num:int):
        self.net_session.cbk_advance_frame(self.cbk_advance_frame)
        self.net_session.cbk_on_event(self.cbk_on_event)

        self.net_session.start()


        self.net_session.set_disconnect_timeout(DISCONNECT_TIMEOUT_MS)
        self.net_session.set_disconnect_notify_start(DISCONNECT_NOTIFY_START_MS)

        self.player   = ggpo_py.GGPOPlayer(own_player_num, ggpo_py.GGPOPlayerType.GGPO_PLAYERTYPE_LOCAL)

        self.opponent = ggpo_py.GGPOPlayer(int(not own_player_num), ggpo_py.GGPOPlayerType.GGPO_PLAYERTYPE_REMOTE)
        self.opponent.player_ip = self.remote_host
        self.opponent.port = self.remote_port

        # We need to take care here, the order matters, we MUST add the lower player number first
        if self.player.player_nr < self.opponent.player_nr:
            self._add_player(self.player)
            self._add_opponent(self.opponent)
        else:
            self._add_opponent(self.opponent)
            self._add_player(self.player)
        
    
    def quit_network(self):
        self.net_session.stop()


    def get_player_input(self):
        inputs = 0
        pressed = pygame.key.get_pressed()

        inputs |= int(pressed[pygame.K_LEFT])
        inputs |= int(pressed[pygame.K_RIGHT])<<1
        inputs |= int(pressed[pygame.K_UP])<<2
        inputs |= int(pressed[pygame.K_DOWN])<<3

        return inputs
    

    def handle_frame(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False

        player_inputs = self.get_player_input()

        if self.net_session.add_local_input(self.player, player_inputs) == ggpo_py.GGPOErrorCode.GGPO_ERRORCODE_SUCCESS:
            result, (all_inputs, disconnected) = self.net_session.synchronize_input()
            if result == ggpo_py.GGPOErrorCode.GGPO_ERRORCODE_SUCCESS:
                self.advance_frame(all_inputs)
        
        self.draw_current_frame()


    def run(self):
        self.running = True
        while self.running:
            self.clock.tick(60)
            self.net_session.idle(5) # Allow GGPO 5ms of working time per frame
            self.handle_frame()

        self.quit_network()


if __name__ == '__main__' and len(sys.argv) == 4:
    local_port = int(sys.argv[1])
    player1 = sys.argv[2].split(":")
    player2 = sys.argv[3].split(":")
    own_player_num = 0

    if player1[0].lower() == "local":
        remote_host, remote_port = player2
        remote_port = int(remote_port)
    else:
        own_player_num = 1
        remote_host, remote_port = player1
        remote_port = int(remote_port)


    pygame.init()
    pygame.display.init()

    window = pygame.display.set_mode((350, 350))

    game = Game(window, "Blocks", local_port, remote_host, remote_port)
    game.init_network(own_player_num)
    game.run()

    pygame.display.quit()
    pygame.quit()