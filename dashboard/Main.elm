module Main exposing (..)

import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (..)
import Http
import Json.Decode exposing (Decoder, float, int, string)
import Json.Decode.Pipeline exposing (decode, hardcoded, optional, required, requiredAt)
import Task


-- application models


type alias Model =
    { user : Maybe User
    , userId : Int
    , isLoading : Bool
    , error : Maybe Http.Error
    }


type alias User =
    { id : Int
    , firstName : String
    , lastName : String
    , avatar : String
    }



-- initial model and commands generator


init : ( Model, Cmd Msg )
init =
    let
        initialModel =
            { user = Nothing
            , userId = 0
            , isLoading = False
            , error = Nothing
            }
    in
        ( initialModel, loadFirstUser )



-- list of possible actions


type Msg
    = LoadUser
    | UserResult (Result Http.Error User)



-- http actions


loadUser : Int -> Cmd Msg
loadUser id =
    let
        url =
            "https://reqres.in/api/users/" ++ (toString id) ++ "?delay=1"

        request =
            Http.get url userDecoder
    in
        Http.send UserResult request


loadFirstUser : Cmd Msg
loadFirstUser =
    Task.perform identity (Task.succeed LoadUser)



-- http json response decoders for a response like
-- {"data":{"id":1,"first_name":"george","last_name":"bluth","avatar":"https://s3.amazonaws.com/uifaces/faces/twitter/calebogden/128.jpg"}}


userDecoder : Decoder User
userDecoder =
    decode User
        |> requiredAt [ "data", "id" ] int
        |> requiredAt [ "data", "first_name" ] string
        |> requiredAt [ "data", "last_name" ] string
        |> requiredAt [ "data", "avatar" ] string



-- renders the view based on model


view : Model -> Html Msg
view model =
    div []
        [ h1 [] [ text "Elm HTTP request playground" ]
        , button [ onClick LoadUser ] [ text "Load user" ]
        , viewUser model
        , p [] [ text (toString model) ]
        ]


viewUser : Model -> Html Msg
viewUser model =
    case model.error of
        Just error ->
            viewError error

        Nothing ->
            if model.isLoading then
                viewLoading model
            else
                viewUserInfo model


viewLoading : Model -> Html Msg
viewLoading model =
    div []
        [ h2 [] [ text "Loading user" ]
        , div [] [ text ("Loading user #" ++ (toString model.userId) ++ " info") ]
        ]


viewUserInfo : Model -> Html Msg
viewUserInfo model =
    case model.user of
        Just user ->
            div [ class "user" ]
                [ h2 [] [ text "User info" ]
                , div [ class "name" ] [ text (user.firstName ++ " " ++ user.lastName) ]
                , div [ class "avatar" ] [ img [ src user.avatar ] [] ]
                ]

        Nothing ->
            div [] [ text ("User not loaded") ]


viewError : Http.Error -> Html Msg
viewError error =
    div [ class "error" ]
        [ h2 [] [ text "Error occured" ]
        , div [] [ text (getErrorMessage error) ]
        ]


getErrorMessage : Http.Error -> String
getErrorMessage error =
    case error of
        Http.Timeout ->
            "request timed out"

        Http.NetworkError ->
            "network error occured"

        Http.BadUrl url ->
            "invalid url '" ++ url ++ "' provided"

        Http.BadStatus response ->
            "request failed with " ++ (toString response.status.code) ++ " (" ++ response.status.message ++ ")"

        Http.BadPayload problem response ->
            "got invalid response (" ++ problem ++ ")"



-- updates the model based on given message


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        LoadUser ->
            let
                newUserId =
                    model.userId + 1
            in
                ( { model | userId = newUserId, isLoading = True, error = Nothing }, loadUser newUserId )

        UserResult (Ok loadedUser) ->
            ( { model | user = Just loadedUser, isLoading = False, error = Nothing }, Cmd.none )

        UserResult (Err error) ->
            -- todo: handle error
            ( { model | error = Just error, isLoading = False }, Cmd.none )



-- subscriptions


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.none



-- main program


main : Program Never Model Msg
main =
    Html.program
        { init = init
        , view = view
        , update = update
        , subscriptions = subscriptions
        }
