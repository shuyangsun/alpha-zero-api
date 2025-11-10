#include "alpha-zero-api/defaults/deserializer.h"
#include "alpha-zero-api/defaults/serializer.h"
#include "connect4/game.h"
#include "connect4/serializer.h"

namespace {

using ::alphazero::game::api::DefaultPolicyOutputDeserializer;
using ::alphazero::game::api::DefaultPolicyOutputSerializer;
using ::alphazero::game::api::test::C4Action;
using ::alphazero::game::api::test::C4Board;
using ::alphazero::game::api::test::C4Player;
using ::alphazero::game::api::test::C4Serializer;

}  // namespace

int main() {
  C4Serializer serializer;
  DefaultPolicyOutputSerializer<C4Board, C4Action, C4Player> po_serializer;
  DefaultPolicyOutputDeserializer<C4Board, C4Action, C4Player> po_deserializer;

  return 0;
}
