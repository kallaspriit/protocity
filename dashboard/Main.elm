module Main exposing (..)

import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (..)


-- main application model


type alias Model =
    { user : Maybe User
    }



-- represents a user


type alias User =
    { idx : Int
    , firstName : String
    , lastName : String
    , avatar : String
    }



-- list of possible messages


type Msg
    = Test


initialModel : Model
initialModel =
    { user = Nothing
    }



-- updates the model based on given message


update : Msg -> Model -> Model
update msg model =
    case msg of
        Test ->
            model


view : Model -> Html msg
view model =
    div []
        [ h1 [] [ text "Hello elm!" ]
        ]


main : Program Never Model Msg
main =
    Html.beginnerProgram
        { model = initialModel
        , view = view
        , update = update
        }
