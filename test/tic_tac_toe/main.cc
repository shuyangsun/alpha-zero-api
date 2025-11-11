#include "alpha-zero-api/defaults/deserializer.h"
#include "alpha-zero-api/defaults/serializer.h"
#include "tic_tac_toe/game.h"
#include "tic_tac_toe/serializer.h"

namespace {

using ::alphazero::game::api::DefaultPolicyOutputDeserializer;
using ::alphazero::game::api::DefaultPolicyOutputSerializer;
using ::alphazero::game::api::test::TttAction;
using ::alphazero::game::api::test::TttBoard;
using ::alphazero::game::api::test::TttGame;
using ::alphazero::game::api::test::TttPlayer;
using ::alphazero::game::api::test::TttSerializer;

}  // namespace

int main() {
  TttGame game;
  TttSerializer serializer;
  DefaultPolicyOutputSerializer<TttBoard, TttAction, TttPlayer> po_serializer;
  DefaultPolicyOutputDeserializer<TttBoard, TttAction, TttPlayer>
      po_deserializer;

  return 0;
}
