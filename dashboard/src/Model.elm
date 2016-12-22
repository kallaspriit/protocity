module Model exposing (..)

import Http exposing (Error)


type alias Flags =
    { swapCount : Int
    }


type alias Model =
    { user : Maybe User
    , userId : Int
    , isLoading : Bool
    , error : Maybe Http.Error
    , swapCount : Int
    }


type alias User =
    { id : Int
    , firstName : String
    , lastName : String
    , avatar : String
    }
